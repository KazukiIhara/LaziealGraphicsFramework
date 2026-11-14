static const uint InvalidIndex = 0xffffffffu;

struct GpuPhysicsParticleData
{
    float3 position;
    float radius;
    float3 previousPosition;
    float inverseMass;
    float3 velocity;
    uint generation;
    float3 force;
    uint alive;
    float restitution;
    float friction;
    uint enabled;
	uint renderGroupIndex;
};

struct GpuPhysicsParticleCommand
{
    uint type;
    uint index;
    uint generation;
    uint boolValue;
    float3 vectorValue;
    float scalarValue;
    float restitution;
    float friction;
	uint renderGroupIndex;
	uint padding;
};

struct GpuPhysicsParticleCommandRange
{
    uint firstCommand;
    uint commandCount;
};

struct GpuPhysicsCollider
{
    uint type;
    uint3 padding0;
    float4 data0;
    float4 data1;
    float4 data2;
    float4 data3;
    float3 referencePosition;
    float restitution;
    float3 linearVelocity;
    float friction;
    float3 angularVelocity;
    float padding1;
};

struct GpuPhysicsParticleCounters
{
    uint aliveCount;
    uint candidatePairCount;
    uint retiredCount;
    uint padding;
};

struct GpuRetiredPhysicsParticle
{
    uint index;
    uint generation;
};

cbuffer GpuPhysicsParticleConstants : register(b0)
{
    float3 gWorldMinimum;
    float gDeltaTime;
    float3 gWorldMaximum;
    float gDamping;
    float3 gGravity;
    float gMaximumCorrection;
    uint gMaximumParticleCount;
    uint gHashBucketCount;
    uint gColliderCount;
    uint gCommandCount;
    uint gSubstepIndex;
    uint gSolverIterationCount;
    uint gCommandRangeCount;
    uint gPadding0;
    float gCellSize;
    float gPenetrationSlop;
    float gPositionCorrectionRate;
    float gBounceVelocityThreshold;
};

float3 SafeNormal(float3 value, float3 fallback)
{
    const float lengthSq = dot(value, value);
    return lengthSq > 1.0e-12f ? value * rsqrt(lengthSq) : fallback;
}

uint HashCell(int3 cell)
{
    uint hash = asuint(cell.x) * 0x8da6b343u;
    hash ^= asuint(cell.y) * 0xd8163841u;
    hash ^= asuint(cell.z) * 0xcb1ab31fu;
    hash ^= hash >> 16u;
    return hash & (gHashBucketCount - 1u);
}

float3 RotateByQuaternion(float3 value, float4 quaternion)
{
    const float4 q = quaternion * rsqrt(max(dot(quaternion, quaternion), 1.0e-12f));
    return value + 2.0f * cross(q.xyz, cross(q.xyz, value) + q.w * value);
}

float3 InverseRotateByQuaternion(float3 value, float4 quaternion)
{
    return RotateByQuaternion(value, float4(-quaternion.xyz, quaternion.w));
}

float3 PerpendicularTo(float3 axis)
{
    const float3 reference = abs(axis.y) < 0.9f ? float3(0.0f, 1.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);
    return SafeNormal(cross(axis, reference), float3(1.0f, 0.0f, 0.0f));
}

float3 ClosestPointOnCircle(float3 queryPosition, float3 center, float3 axis, float radius)
{
    const float3 offset = queryPosition - center;
    const float3 radial = offset - axis * dot(offset, axis);
    return center + SafeNormal(radial, PerpendicularTo(axis)) * radius;
}

float3 ClosestPointOnShellCapSurface(
    float3 queryPosition,
    float3 sphereCenter,
    float3 axis,
    float openingOffset,
    float radius,
    float openingRadius)
{
    const float3 direction = SafeNormal(queryPosition - sphereCenter, -axis);
    const float3 spherePoint = sphereCenter + direction * radius;
    if (dot(spherePoint - sphereCenter, axis) <= openingOffset)
    {
        return spherePoint;
    }
    return ClosestPointOnCircle(queryPosition, sphereCenter + axis * openingOffset, axis, openingRadius);
}

bool FindColliderContact(
    float3 center,
    float radius,
    GpuPhysicsCollider collider,
    out float3 contactPoint,
    out float3 normal,
    out float penetration)
{
    contactPoint = center;
    normal = float3(0.0f, 1.0f, 0.0f);
    penetration = 0.0f;

    if (collider.type == 0u)
    {
        const float3 offset = center - collider.data0.xyz;
        const float minimumDistance = radius + collider.data0.w;
        const float distanceSq = dot(offset, offset);
        if (distanceSq > minimumDistance * minimumDistance)
        {
            return false;
        }
        const float distance = sqrt(max(distanceSq, 0.0f));
        normal = SafeNormal(offset, float3(0.0f, 1.0f, 0.0f));
        contactPoint = collider.data0.xyz + normal * collider.data0.w;
        penetration = minimumDistance - distance;
        return true;
    }

    if (collider.type == 1u)
    {
        const float3 axis = SafeNormal(collider.data1.xyz, float3(0.0f, 1.0f, 0.0f));
        const float baseRadius = collider.data0.w;
        const float height = collider.data1.w;
        const float sphereRadius = (baseRadius * baseRadius + height * height) / (2.0f * height);
        const float3 sphereCenter = collider.data0.xyz + axis * sphereRadius;
        const float3 centerOffset = center - sphereCenter;
        const float centerDistance = length(centerOffset);
        const float3 surfaceDirection = SafeNormal(centerOffset, -axis);
        const float3 surfacePoint = sphereCenter + surfaceDirection * sphereRadius;
        const float surfaceHeight = dot(surfacePoint - collider.data0.xyz, axis);
        if (surfaceHeight < -1.0e-6f || surfaceHeight > height + 1.0e-6f)
        {
            return false;
        }
        penetration = centerDistance + radius - sphereRadius;
        if (penetration < 0.0f)
        {
            return false;
        }
        contactPoint = surfacePoint;
        normal = -surfaceDirection;
        return true;
    }

    if (collider.type == 2u)
    {
        const float3 axis = SafeNormal(collider.data1.xyz, float3(0.0f, 1.0f, 0.0f));
        const float outerRadius = collider.data0.w;
        const float innerRadius = collider.data1.w;
        const float height = collider.data2.x;
        const float3 sphereCenter = collider.data0.xyz + axis * outerRadius;
        const float openingOffset = height - outerRadius;
        const float outerOpeningRadius = sqrt(max(outerRadius * outerRadius - openingOffset * openingOffset, 0.0f));
        const float innerOpeningRadius = sqrt(max(innerRadius * innerRadius - openingOffset * openingOffset, 0.0f));
        const float3 ringCenter = sphereCenter + axis * openingOffset;
        const float3 ringOffset = center - ringCenter;
        const float3 radial = ringOffset - axis * dot(ringOffset, axis);
        const float radialLength = length(radial);
        const float3 radialDirection = SafeNormal(radial, PerpendicularTo(axis));
        const float3 candidates[3] = {
            ClosestPointOnShellCapSurface(center, sphereCenter, axis, openingOffset, outerRadius, outerOpeningRadius),
            ClosestPointOnShellCapSurface(center, sphereCenter, axis, openingOffset, innerRadius, innerOpeningRadius),
            ringCenter + radialDirection * clamp(radialLength, innerOpeningRadius, outerOpeningRadius)
        };
        contactPoint = candidates[0];
        float distanceSq = dot(contactPoint - center, contactPoint - center);
        [unroll]
        for (uint candidateIndex = 1u; candidateIndex < 3u; ++candidateIndex)
        {
            const float candidateDistanceSq = dot(candidates[candidateIndex] - center, candidates[candidateIndex] - center);
            if (candidateDistanceSq < distanceSq)
            {
                contactPoint = candidates[candidateIndex];
                distanceSq = candidateDistanceSq;
            }
        }
        const float3 centerOffset = center - sphereCenter;
        const float centerDistanceSq = dot(centerOffset, centerOffset);
        const bool centerInside = dot(centerOffset, axis) <= openingOffset &&
            centerDistanceSq <= outerRadius * outerRadius && centerDistanceSq >= innerRadius * innerRadius;
        if (!centerInside && distanceSq > radius * radius)
        {
            return false;
        }
        const float distance = sqrt(max(distanceSq, 0.0f));
        const float3 contactNormal = SafeNormal(contactPoint - center, centerInside ? axis : -axis);
        normal = centerInside ? contactNormal : -contactNormal;
        penetration = centerInside ? radius + distance : radius - distance;
        return true;
    }

    if (collider.type == 3u)
    {
        const float3 minimum = collider.data0.xyz;
        const float3 maximum = collider.data1.xyz;
        contactPoint = clamp(center, minimum, maximum);
        const float3 offset = center - contactPoint;
        const float distanceSq = dot(offset, offset);
        if (distanceSq > radius * radius)
        {
            return false;
        }
        if (distanceSq > 1.0e-12f)
        {
            const float distance = sqrt(distanceSq);
            normal = offset / distance;
            penetration = radius - distance;
            return true;
        }
        const float distances[6] = {
            center.x - minimum.x, maximum.x - center.x,
            center.y - minimum.y, maximum.y - center.y,
            center.z - minimum.z, maximum.z - center.z
        };
        const float3 normals[6] = {
            float3(-1.0f, 0.0f, 0.0f), float3(1.0f, 0.0f, 0.0f),
            float3(0.0f, -1.0f, 0.0f), float3(0.0f, 1.0f, 0.0f),
            float3(0.0f, 0.0f, -1.0f), float3(0.0f, 0.0f, 1.0f)
        };
        uint nearest = 0u;
        [unroll]
        for (uint i = 1u; i < 6u; ++i)
        {
            nearest = distances[i] < distances[nearest] ? i : nearest;
        }
        normal = normals[nearest];
        contactPoint = center + normal * distances[nearest];
        penetration = radius + distances[nearest];
        return true;
    }

    const float3 localCenter = InverseRotateByQuaternion(center - collider.data0.xyz, collider.data2);
    const float3 extents = collider.data1.xyz;
    const float3 localClosest = clamp(localCenter, -extents, extents);
    const float3 localOffset = localCenter - localClosest;
    const float distanceSq = dot(localOffset, localOffset);
    if (distanceSq > radius * radius)
    {
        return false;
    }
    if (distanceSq > 1.0e-12f)
    {
        const float distance = sqrt(distanceSq);
        normal = RotateByQuaternion(localOffset / distance, collider.data2);
        contactPoint = collider.data0.xyz + RotateByQuaternion(localClosest, collider.data2);
        penetration = radius - distance;
        return true;
    }
    const float distances[6] = {
        localCenter.x + extents.x, extents.x - localCenter.x,
        localCenter.y + extents.y, extents.y - localCenter.y,
        localCenter.z + extents.z, extents.z - localCenter.z
    };
    const float3 localNormals[6] = {
        float3(-1.0f, 0.0f, 0.0f), float3(1.0f, 0.0f, 0.0f),
        float3(0.0f, -1.0f, 0.0f), float3(0.0f, 1.0f, 0.0f),
        float3(0.0f, 0.0f, -1.0f), float3(0.0f, 0.0f, 1.0f)
    };
    uint nearest = 0u;
    [unroll]
    for (uint i = 1u; i < 6u; ++i)
    {
        nearest = distances[i] < distances[nearest] ? i : nearest;
    }
    normal = RotateByQuaternion(localNormals[nearest], collider.data2);
    contactPoint = center + normal * distances[nearest];
    penetration = radius + distances[nearest];
    return true;
}

float3 ColliderSurfaceVelocity(GpuPhysicsCollider collider, float3 contactPoint)
{
    return collider.linearVelocity + cross(collider.angularVelocity, contactPoint - collider.referencePosition);
}

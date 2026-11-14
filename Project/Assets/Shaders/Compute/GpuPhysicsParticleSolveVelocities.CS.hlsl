#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);
RWStructuredBuffer<float4> gCorrections : register(u1);
RWStructuredBuffer<uint> gBucketHeads : register(u2);
RWStructuredBuffer<int4> gParticleCells : register(u3);
RWStructuredBuffer<uint> gParticleNext : register(u4);
StructuredBuffer<GpuPhysicsCollider> gColliders : register(t0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount)
    {
        return;
    }
    const GpuPhysicsParticleData particle = gParticles[id.x];
    if (particle.alive == 0u)
    {
        gCorrections[id.x] = 0.0f;
        return;
    }
    if (particle.enabled == 0u)
    {
        gCorrections[id.x] = float4(particle.velocity, 0.0f);
        return;
    }
    if (particle.inverseMass <= 0.0f)
    {
        gCorrections[id.x] = 0.0f;
        return;
    }

    float3 velocity = particle.velocity * gDamping;
    const int3 centerCell = gParticleCells[id.x].xyz;
    [unroll]
    for (int z = -1; z <= 1; ++z)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            [unroll]
            for (int x = -1; x <= 1; ++x)
            {
                const int3 neighborCell = centerCell + int3(x, y, z);
                uint otherIndex = gBucketHeads[HashCell(neighborCell)];
                while (otherIndex != InvalidIndex)
                {
                    const uint nextIndex = gParticleNext[otherIndex];
                    if (otherIndex != id.x && all(gParticleCells[otherIndex].xyz == neighborCell))
                    {
                        const GpuPhysicsParticleData other = gParticles[otherIndex];
                        const float3 delta = other.position - particle.position;
                        const float minimumDistance = particle.radius + other.radius;
                        const float inverseMassSum = particle.inverseMass + other.inverseMass;
                        if (other.alive != 0u && other.enabled != 0u && inverseMassSum > 0.0f &&
                            dot(delta, delta) <= minimumDistance * minimumDistance + 1.0e-6f)
                        {
                            const float3 normal = SafeNormal(delta, float3(1.0f, 0.0f, 0.0f));
                            const float3 otherVelocity = other.velocity * gDamping;
                            const float3 relativeVelocity = otherVelocity - velocity;
                            const float normalSpeed = dot(relativeVelocity, normal);
                            if (normalSpeed < 0.0f)
                            {
                                const float restitution = -normalSpeed > gBounceVelocityThreshold
                                    ? max(particle.restitution, other.restitution) : 0.0f;
                                const float impulseMagnitude = -(1.0f + restitution) * normalSpeed / inverseMassSum;
                                velocity -= normal * impulseMagnitude * particle.inverseMass;
                                const float3 tangentVelocity = relativeVelocity - normal * normalSpeed;
                                const float tangentSpeed = length(tangentVelocity);
                                if (tangentSpeed > 1.0e-6f)
                                {
                                    const float friction = sqrt(particle.friction * other.friction);
                                    const float tangentImpulse = min(tangentSpeed / inverseMassSum, friction * impulseMagnitude);
                                    velocity += tangentVelocity / tangentSpeed * tangentImpulse * particle.inverseMass;
                                }
                            }
                        }
                    }
                    otherIndex = nextIndex;
                }
            }
        }
    }

    const uint colliderOffset = gSubstepIndex * gColliderCount;
    for (uint colliderIndex = 0u; colliderIndex < gColliderCount; ++colliderIndex)
    {
        const GpuPhysicsCollider collider = gColliders[colliderOffset + colliderIndex];
        float3 contactPoint;
        float3 normal;
        float penetration;
        if (!FindColliderContact(particle.position, particle.radius + gPenetrationSlop, collider, contactPoint, normal, penetration))
        {
            continue;
        }
        const float3 surfaceVelocity = ColliderSurfaceVelocity(collider, contactPoint);
        const float3 relativeVelocity = velocity - surfaceVelocity;
        const float normalSpeed = dot(relativeVelocity, normal);
        if (normalSpeed >= 0.0f)
        {
            continue;
        }
        const float3 normalVelocity = normal * normalSpeed;
        const float3 tangentVelocity = relativeVelocity - normalVelocity;
        const float restitution = -normalSpeed > gBounceVelocityThreshold
            ? max(particle.restitution, collider.restitution) : 0.0f;
        const float friction = sqrt(particle.friction * collider.friction);
        velocity = surfaceVelocity + tangentVelocity * (1.0f - friction) - normalVelocity * restitution;
    }
    gCorrections[id.x] = float4(velocity, 0.0f);
}

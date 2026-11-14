#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);
RWStructuredBuffer<float4> gCorrections : register(u1);
RWStructuredBuffer<uint> gBucketHeads : register(u2);
RWStructuredBuffer<int4> gParticleCells : register(u3);
RWStructuredBuffer<uint> gParticleNext : register(u4);
StructuredBuffer<GpuPhysicsCollider> gColliders : register(t0);
RWStructuredBuffer<GpuPhysicsParticleCounters> gCounters : register(u5);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount)
    {
        return;
    }
    const GpuPhysicsParticleData particle = gParticles[id.x];
    if (particle.alive == 0u || particle.enabled == 0u || particle.inverseMass <= 0.0f)
    {
        gCorrections[id.x] = 0.0f;
        return;
    }

    float3 correction = 0.0f;
    uint correctionCount = 0u;
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
                        if (other.alive != 0u && other.enabled != 0u)
                        {
                            if (otherIndex > id.x)
                            {
                                InterlockedAdd(gCounters[0].candidatePairCount, 1u);
                            }
                            const float3 delta = other.position - particle.position;
                            const float minimumDistance = particle.radius + other.radius;
                            const float distanceSq = dot(delta, delta);
                            const float inverseMassSum = particle.inverseMass + other.inverseMass;
                            if (distanceSq < minimumDistance * minimumDistance && inverseMassSum > 0.0f)
                            {
                                const float3 fallback = ((id.x + otherIndex) % 3u == 0u)
                                    ? float3(1.0f, 0.0f, 0.0f)
                                    : (((id.x + otherIndex) % 3u == 1u)
                                        ? float3(0.0f, 1.0f, 0.0f)
                                        : float3(0.0f, 0.0f, 1.0f));
                                const float3 normal = SafeNormal(delta, fallback);
                                const float distance = sqrt(max(distanceSq, 0.0f));
                                const float penetration = max(minimumDistance - distance - gPenetrationSlop, 0.0f);
                                const float depth = min(penetration * gPositionCorrectionRate, gMaximumCorrection);
                                correction -= normal * depth * particle.inverseMass / inverseMassSum;
                                ++correctionCount;
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
        float3 contactPoint;
        float3 normal;
        float penetration;
        if (FindColliderContact(
            particle.position,
            particle.radius,
            gColliders[colliderOffset + colliderIndex],
            contactPoint,
            normal,
            penetration))
        {
            const float depth = min(max(penetration - gPenetrationSlop, 0.0f) * gPositionCorrectionRate, gMaximumCorrection);
            correction += normal * depth;
            ++correctionCount;
        }
    }

    if (correctionCount > 1u)
    {
        correction /= (float)correctionCount;
    }
    const float correctionLength = length(correction);
    if (correctionLength > gMaximumCorrection && correctionLength > 0.0f)
    {
        correction *= gMaximumCorrection / correctionLength;
    }
    gCorrections[id.x] = float4(correction, 0.0f);
}

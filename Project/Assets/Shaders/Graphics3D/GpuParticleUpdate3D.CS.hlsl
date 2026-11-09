#include "GpuParticle3D.hlsli"

RWStructuredBuffer<GpuParticleData> gParticles : register(u0);
RWStructuredBuffer<uint> gFreeList : register(u1);
RWStructuredBuffer<GpuParticleCounters> gCounters : register(u2);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount || gParticles[id.x].alive == 0u)
    {
        return;
    }

    GpuParticleData particle = gParticles[id.x];
    particle.age += gDeltaTime;
    bool expired = particle.age >= particle.lifetime;
    if (!expired)
    {
        particle.position += particle.velocity * gDeltaTime +
            particle.acceleration * (0.5f * gDeltaTime * gDeltaTime);
        particle.velocity += particle.acceleration * gDeltaTime;
        particle.rotation += particle.angularVelocity * gDeltaTime;
    }

    const float3 minimum = gWorldMinimum - gBoundsPadding;
    const float3 maximum = gWorldMaximum + gBoundsPadding;
    const bool finite = all(isfinite(particle.position)) && all(isfinite(particle.velocity)) &&
        isfinite(particle.rotation);
    const bool outside = !finite || (gRemoveOutOfBounds != 0u &&
        (any(particle.position < minimum) || any(particle.position > maximum)));
    if (!expired && !outside)
    {
        gParticles[id.x] = particle;
        return;
    }

    particle.alive = 0u;
    gParticles[id.x] = particle;
    uint freeIndex;
    InterlockedAdd(gCounters[0].freeCount, 1u, freeIndex);
    gFreeList[freeIndex] = id.x;
    InterlockedAdd(gCounters[0].aliveCount, 0xffffffffu);
    if (expired)
    {
        InterlockedAdd(gCounters[0].expiredCount, 1u);
    }
    else
    {
        InterlockedAdd(gCounters[0].outOfBoundsCount, 1u);
    }
}

#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);
RWStructuredBuffer<GpuPhysicsParticleCounters> gCounters : register(u1);
RWStructuredBuffer<GpuRetiredPhysicsParticle> gRetiredParticles : register(u2);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount || gParticles[id.x].alive == 0u)
    {
        return;
    }
    GpuPhysicsParticleData particle = gParticles[id.x];
    particle.force = 0.0f;
    const bool finite = all(isfinite(particle.position)) && all(isfinite(particle.velocity));
    const bool outside = any(particle.position < gWorldMinimum) || any(particle.position > gWorldMaximum);
    if (finite && !outside)
    {
        gParticles[id.x] = particle;
        return;
    }
    particle.alive = 0u;
    gParticles[id.x] = particle;
    uint retiredIndex;
    InterlockedAdd(gCounters[0].retiredCount, 1u, retiredIndex);
    if (retiredIndex < gMaximumParticleCount)
    {
        gRetiredParticles[retiredIndex].index = id.x;
        gRetiredParticles[retiredIndex].generation = particle.generation;
    }
    InterlockedAdd(gCounters[0].aliveCount, 0xffffffffu);
}

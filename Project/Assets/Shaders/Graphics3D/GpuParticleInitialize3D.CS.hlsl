#include "GpuParticle3D.hlsli"

RWStructuredBuffer<GpuParticleData> gParticles : register(u0);
RWStructuredBuffer<uint> gFreeList : register(u1);
RWStructuredBuffer<GpuParticleCounters> gCounters : register(u2);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount)
    {
        return;
    }

    GpuParticleData particle = (GpuParticleData)0;
    particle.textureIndex = 0xffffffffu;
    gParticles[id.x] = particle;
    gFreeList[id.x] = id.x;

    if (id.x == 0u)
    {
        GpuParticleCounters counters = (GpuParticleCounters)0;
        counters.freeCount = gMaximumParticleCount;
        gCounters[0] = counters;
    }
}

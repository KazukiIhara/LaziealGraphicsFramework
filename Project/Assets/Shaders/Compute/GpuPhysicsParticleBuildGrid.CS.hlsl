#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);
RWStructuredBuffer<uint> gBucketHeads : register(u1);
RWStructuredBuffer<int4> gParticleCells : register(u2);
RWStructuredBuffer<uint> gParticleNext : register(u3);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount || gParticles[id.x].alive == 0u || gParticles[id.x].enabled == 0u)
    {
        return;
    }
    const int3 cell = int3(floor(gParticles[id.x].position / gCellSize));
    const uint bucket = HashCell(cell);
    uint previous;
    InterlockedExchange(gBucketHeads[bucket], id.x, previous);
    gParticleCells[id.x] = int4(cell, 0);
    gParticleNext[id.x] = previous;
}

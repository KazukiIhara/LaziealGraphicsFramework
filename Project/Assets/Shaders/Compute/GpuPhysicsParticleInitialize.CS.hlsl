#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);
RWStructuredBuffer<uint> gBucketHeads : register(u1);
RWStructuredBuffer<GpuPhysicsParticleCounters> gCounters : register(u2);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x < gMaximumParticleCount)
    {
        GpuPhysicsParticleData particle = (GpuPhysicsParticleData)0;
        gParticles[id.x] = particle;
    }
    if (id.x < gHashBucketCount)
    {
        gBucketHeads[id.x] = InvalidIndex;
    }
    if (id.x == 0u)
    {
        gCounters[0] = (GpuPhysicsParticleCounters)0;
    }
}

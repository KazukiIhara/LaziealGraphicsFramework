#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleCounters> gCounters : register(u0);

[numthreads(1, 1, 1)]
void main()
{
    gCounters[0].candidatePairCount = 0u;
    gCounters[0].retiredCount = 0u;
}

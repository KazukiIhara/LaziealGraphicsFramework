#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<uint> gBucketHeads : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x < gHashBucketCount)
    {
        gBucketHeads[id.x] = InvalidIndex;
    }
}

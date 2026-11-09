#include "GpuParticle3D.hlsli"

RWStructuredBuffer<GpuParticleCounters> gCounters : register(u0);

[numthreads(1, 1, 1)]
void main()
{
    // freeCountとaliveCountはフレームを跨ぐため、ステップ統計だけを初期化する。
    gCounters[0].generatedCount = 0u;
    gCounters[0].rejectedCount = 0u;
    gCounters[0].expiredCount = 0u;
    gCounters[0].outOfBoundsCount = 0u;
}

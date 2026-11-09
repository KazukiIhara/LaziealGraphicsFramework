#include "GpuParticle3D.hlsli"

RWStructuredBuffer<GpuParticleData> gParticles : register(u0);
RWStructuredBuffer<uint> gFreeList : register(u1);
RWStructuredBuffer<GpuParticleCounters> gCounters : register(u2);
StructuredBuffer<GpuParticleSpawnBatch> gSpawnBatches : register(t0);

[numthreads(64, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadId : SV_GroupThreadID)
{
    const uint batchIndex = groupId.x;
    if (batchIndex >= gSpawnBatchCount)
    {
        return;
    }

    GpuParticleSpawnBatch batch = gSpawnBatches[batchIndex];
    for (uint particleInBatch = threadId.x; particleInBatch < batch.count; particleInBatch += 64u)
    {
        uint previousFreeCount;
        for (;;)
        {
            previousFreeCount = gCounters[0].freeCount;
            if (previousFreeCount == 0u)
            {
                break;
            }
            uint observedFreeCount;
            InterlockedCompareExchange(
                gCounters[0].freeCount,
                previousFreeCount,
                previousFreeCount - 1u,
                observedFreeCount);
            if (observedFreeCount == previousFreeCount)
            {
                break;
            }
        }
        if (previousFreeCount == 0u)
        {
            InterlockedAdd(gCounters[0].rejectedCount, 1u);
            continue;
        }

        const uint particleIndex = gFreeList[previousFreeCount - 1u];
        const uint seed = Hash(batch.randomSeed ^ particleInBatch);
        GpuParticleData particle = (GpuParticleData)0;
        particle.position = batch.position;
        particle.rotation = RandomRange(batch.minimumRotation, batch.maximumRotation, seed + 0u);
        particle.velocity = float3(
            RandomRange(batch.minimumVelocity.x, batch.maximumVelocity.x, seed + 1u),
            RandomRange(batch.minimumVelocity.y, batch.maximumVelocity.y, seed + 2u),
            RandomRange(batch.minimumVelocity.z, batch.maximumVelocity.z, seed + 3u));

        particle.acceleration = batch.acceleration;
        particle.lifetime = RandomRange(batch.minimumLifetime, batch.maximumLifetime, seed + 4u);
        particle.startColor = batch.startColor;
        particle.endColor = batch.endColor;
        particle.startSize = batch.startSize;
        particle.endSize = batch.endSize;
        particle.angularVelocity = RandomRange(
            batch.minimumAngularVelocity,
            batch.maximumAngularVelocity,
            seed + 5u);
        particle.textureIndex = batch.textureIndex;
        particle.alive = 1u;
        gParticles[particleIndex] = particle;
        InterlockedAdd(gCounters[0].aliveCount, 1u);
        InterlockedAdd(gCounters[0].generatedCount, 1u);
    }
}

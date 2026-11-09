struct GpuParticleData
{
    float3 position;
    float rotation;
    float3 velocity;
    float age;
    float3 acceleration;
    float lifetime;
    float4 startColor;
    float4 endColor;
    float2 startSize;
    float2 endSize;
    float angularVelocity;
    uint textureIndex;
    uint alive;
    float padding;
};

struct GpuParticleSpawnBatch
{
    float3 position;
    float minimumLifetime;
    float3 minimumVelocity;
    float maximumLifetime;
    float3 maximumVelocity;
    float minimumRotation;
    float3 acceleration;
    float maximumRotation;
    float4 startColor;
    float4 endColor;
    float2 startSize;
    float2 endSize;
    float minimumAngularVelocity;
    float maximumAngularVelocity;
    uint count;
    uint randomSeed;
    uint textureIndex;
    uint3 padding;
};

struct GpuParticleCounters
{
    uint freeCount;
    uint aliveCount;
    uint generatedCount;
    uint rejectedCount;
    uint expiredCount;
    uint outOfBoundsCount;
};

struct GpuParticleDispatchArguments
{
    uint threadGroupCountX;
    uint threadGroupCountY;
    uint threadGroupCountZ;
};

cbuffer GpuParticleConstants : register(b0)
{
    float3 gWorldMinimum;
    float gDeltaTime;
    float3 gWorldMaximum;
    float gBoundsPadding;
    uint gMaximumParticleCount;
    uint gSpawnBatchCount;
    uint gRemoveOutOfBounds;
    uint gPadding;
};

uint Hash(uint value)
{
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    value *= 0x846ca68bu;
    return value ^ (value >> 16);
}

float Random01(uint seed)
{
    return (Hash(seed) & 0x00ffffffu) / 16777215.0f;
}

float RandomRange(float minimum, float maximum, uint seed)
{
    return lerp(minimum, maximum, Random01(seed));
}

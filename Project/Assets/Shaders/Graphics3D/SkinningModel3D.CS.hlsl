struct ModelVertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float4 tangent;
};

struct ModelVertexInfluence
{
    uint4 boneIndices;
    float4 weights;
};

struct SkinningPaletteEntry
{
    float4x4 positionMatrix;
    float4x4 normalMatrix;
};

struct SkinningData
{
    uint vertexCount;
    uint3 padding;
};

ConstantBuffer<SkinningData> gSkinning : register(b0);
StructuredBuffer<SkinningPaletteEntry> gPalette : register(t0);
StructuredBuffer<ModelVertex> gInputVertices : register(t1);
StructuredBuffer<ModelVertexInfluence> gInfluences : register(t2);
RWStructuredBuffer<ModelVertex> gOutputVertices : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint vertexIndex = dispatchThreadId.x;
    if (vertexIndex >= gSkinning.vertexCount)
    {
        return;
    }

    const ModelVertex input = gInputVertices[vertexIndex];
    const ModelVertexInfluence influence = gInfluences[vertexIndex];
    float3 position = 0.0f;
    float3 normal = 0.0f;
    float3 tangent = 0.0f;
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        const float weight = influence.weights[i];
        const SkinningPaletteEntry palette = gPalette[influence.boneIndices[i]];
        position += mul(float4(input.position, 1.0f), palette.positionMatrix).xyz * weight;
        normal += mul(float4(input.normal, 0.0f), palette.normalMatrix).xyz * weight;
        tangent += mul(float4(input.tangent.xyz, 0.0f), palette.positionMatrix).xyz * weight;
    }

    normal = normalize(normal);
    tangent -= normal * dot(normal, tangent);
    const float tangentLengthSq = dot(tangent, tangent);
    if (tangentLengthSq > 1.0e-8f)
    {
        tangent *= rsqrt(tangentLengthSq);
    }
    ModelVertex output = input;
    output.position = position;
    output.normal = normal;
    output.tangent = float4(tangent, input.tangent.w);
    gOutputVertices[vertexIndex] = output;
}

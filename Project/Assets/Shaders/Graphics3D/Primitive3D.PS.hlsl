#include "Graphics3D.hlsli"

SamplerState gBaseColorSampler : register(s0);

struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
};

GBufferOutput main(MSOutput3D input)
{
    GBufferOutput output;

    // Albedoの出力
    output.albedo = input.baseColor;
    float2 uvDx = ddx(input.uv);
    float2 uvDy = ddy(input.uv);
    if (input.textureIndex != 0xffffffffu)
    {
        Texture2D<float4> baseColorTexture = ResourceDescriptorHeap[NonUniformResourceIndex(input.textureIndex)];
        output.albedo *= baseColorTexture.SampleGrad(gBaseColorSampler, input.uv, uvDx, uvDy);
    }

    // Normalの出力
    output.normal = float4(normalize(input.normal), 1.0f);

    return output;
}

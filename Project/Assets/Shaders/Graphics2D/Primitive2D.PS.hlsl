#include "Graphics2D.hlsli"

static const uint kInvalidTextureIndex = 0xffffffffu;

SamplerState gTextureSampler : register(s0);

float4 main(MSOutput2D input) : SV_Target
{
    float4 output;

    output = input.baseColor;
    if (input.textureIndex != kInvalidTextureIndex)
    {
        Texture2D<float4> textureData =
            ResourceDescriptorHeap[NonUniformResourceIndex(input.textureIndex)];
        output *= textureData.Sample(gTextureSampler, input.uv);
    }

    return output;
}

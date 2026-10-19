#include "Graphics2D.hlsli"

SamplerState gTextureSampler : register(s0);

float4 main(MSOutput2D input) : SV_Target
{
    float4 output;

    output = input.baseColor;
    if (input.textureIndex != 0xffffffffu)
    {
        Texture2D<float4> textureData =
            ResourceDescriptorHeap[NonUniformResourceIndex(input.textureIndex)];
        output *= textureData.Sample(gTextureSampler, input.uv);
    }

    return output;
}

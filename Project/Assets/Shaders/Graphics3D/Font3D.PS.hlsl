#include "Font3D.hlsli"

SamplerState gFontSampler : register(s0);

float4 main(FontGlyphOutput input) : SV_Target0
{
    Texture2D<float4> texture =
        ResourceDescriptorHeap[NonUniformResourceIndex(input.textureIndex)];
    float4 color = input.color * texture.Sample(gFontSampler, input.uv);
    clip(color.a - (1.0f / 255.0f));
    return color;
}

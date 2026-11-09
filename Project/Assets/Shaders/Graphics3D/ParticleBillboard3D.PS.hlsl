#include "ParticleBillboard3D.hlsli"

SamplerState gParticleSampler : register(s0);

float4 main(ParticleBillboardOutput input) : SV_Target0
{
    float4 color = input.color;
    if (input.textureIndex != 0xffffffffu)
    {
        Texture2D<float4> texture = ResourceDescriptorHeap[NonUniformResourceIndex(input.textureIndex)];
        color *= texture.Sample(gParticleSampler, input.uv);
    }
    color.a = saturate(color.a);
    return color;
}

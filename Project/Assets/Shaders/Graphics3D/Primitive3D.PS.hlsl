#include "Graphics3D.hlsli"

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

    // Normalの出力
    output.normal = float4(normalize(input.normal), 1.0f);

    return output;
}

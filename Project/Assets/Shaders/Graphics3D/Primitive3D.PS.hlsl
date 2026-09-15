#include "Graphics3D.hlsli"

SamplerState gBaseColorSampler : register(s0);

float3 ApplyNormalMap(MSOutput3D input, float2 uvDx, float2 uvDy)
{
    const float3 surfaceNormal = normalize(input.normal);
    if (input.normalTextureIndex == 0xffffffffu || input.normalScale == 0.0f)
    {
        return surfaceNormal;
    }

    Texture2D<float4> normalTexture =
        ResourceDescriptorHeap[NonUniformResourceIndex(input.normalTextureIndex)];
    float3 tangentNormal =
        normalTexture.SampleGrad(gBaseColorSampler, input.uv, uvDx, uvDy).xyz * 2.0f - 1.0f;
    tangentNormal.xy *= input.normalScale;
    const float tangentNormalLengthSquared = dot(tangentNormal, tangentNormal);
    if (tangentNormalLengthSquared <= 1.0e-12f)
    {
        return surfaceNormal;
    }
    tangentNormal *= rsqrt(tangentNormalLengthSquared);

    float3 tangent =
        input.tangent.xyz - surfaceNormal * dot(surfaceNormal, input.tangent.xyz);
    float tangentLengthSquared = dot(tangent, tangent);
    float3 bitangent;
    if (input.tangent.w != 0.0f && tangentLengthSquared > 1.0e-12f)
    {
        tangent *= rsqrt(tangentLengthSquared);
        bitangent = cross(surfaceNormal, tangent) * input.tangent.w;
    }
    else
    {
        const float3 positionDx = ddx(input.worldPosition);
        const float3 positionDy = ddy(input.worldPosition);
        const float3 positionDyPerpendicular = cross(positionDy, surfaceNormal);
        const float3 positionDxPerpendicular = cross(surfaceNormal, positionDx);
        tangent = positionDyPerpendicular * uvDx.x + positionDxPerpendicular * uvDy.x;
        bitangent = positionDyPerpendicular * uvDx.y + positionDxPerpendicular * uvDy.y;

        const float basisLengthSquared = max(dot(tangent, tangent), dot(bitangent, bitangent));
        if (basisLengthSquared <= 1.0e-12f)
        {
            return surfaceNormal;
        }

        const float inverseBasisLength = rsqrt(basisLengthSquared);
        tangent *= inverseBasisLength;
        bitangent *= inverseBasisLength;
    }

    return normalize(
        tangentNormal.x * tangent +
        tangentNormal.y * bitangent +
        tangentNormal.z * surfaceNormal);
}

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
    output.normal = float4(ApplyNormalMap(input, uvDx, uvDy), 1.0f);

    return output;
}

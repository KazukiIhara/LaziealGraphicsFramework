#include "Graphics3D.hlsli"

struct LineData
{
    InstanceData3D instData;
    float3 start;
    float padding0;
    float3 end;
    float padding1;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<LineData> gLine : register(t0);

[outputtopology("line")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[2],
    out indices uint2 lines[1])
{
    SetMeshOutputCounts(2, 1);

    LineData lineData = gLine[DTid.x];

    float4 startWorldPos = mul(float4(lineData.start, 1.0f), lineData.instData.worldMatrix);
    float4 endWorldPos = mul(float4(lineData.end, 1.0f), lineData.instData.worldMatrix);

    vertices[0].position = mul(startWorldPos, gCamera.viewProj);
    vertices[0].baseColor = lineData.instData.baseColor;
    vertices[0].uv = float2(0, 0.5f) * lineData.instData.uvScale + lineData.instData.uvOffset;
    vertices[0].textureIndex = lineData.instData.textureIndex;
    vertices[0].normalTextureIndex = lineData.instData.normalTextureIndex;
    vertices[0].normalScale = lineData.instData.normalScale;
    vertices[0].worldPosition = startWorldPos.xyz;
    vertices[0].tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
    vertices[0].pbrTextureIndices = uint2(
        lineData.instData.metallicRoughnessTextureIndex,
        lineData.instData.occlusionTextureIndex);
    vertices[0].pbrFactors = float3(
        lineData.instData.metallic,
        lineData.instData.roughness,
        lineData.instData.occlusionStrength);
    vertices[0].normal = float4(
        ToWorldNormal(lineData.instData, float3(0.0f, 0.0f, 1.0f)),
        (lineData.instData.materialFlags & kMaterialUnlit) != 0 ? 1.0f : 0.0f);

    vertices[1].position = mul(endWorldPos, gCamera.viewProj);
    vertices[1].baseColor = lineData.instData.baseColor;
    vertices[1].uv = float2(1, 0.5f) * lineData.instData.uvScale + lineData.instData.uvOffset;
    vertices[1].textureIndex = lineData.instData.textureIndex;
    vertices[1].normalTextureIndex = lineData.instData.normalTextureIndex;
    vertices[1].normalScale = lineData.instData.normalScale;
    vertices[1].worldPosition = endWorldPos.xyz;
    vertices[1].tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
    vertices[1].pbrTextureIndices = uint2(
        lineData.instData.metallicRoughnessTextureIndex,
        lineData.instData.occlusionTextureIndex);
    vertices[1].pbrFactors = float3(
        lineData.instData.metallic,
        lineData.instData.roughness,
        lineData.instData.occlusionStrength);
    vertices[1].normal = float4(
        ToWorldNormal(lineData.instData, float3(0.0f, 0.0f, 1.0f)),
        (lineData.instData.materialFlags & kMaterialUnlit) != 0 ? 1.0f : 0.0f);

    lines[0] = uint2(0, 1);
}

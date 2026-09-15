#include "Graphics3D.hlsli"

struct TriangleData
{
    InstanceData3D instData;
    float3 vertexOffsets[3];
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<TriangleData> gTriangle : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[3],
    out indices uint3 triangles[1])
{
    // 三角形1枚分を出力
    SetMeshOutputCounts(3, 1);

    // インデックスから三角形データを取得
    TriangleData tri = gTriangle[DTid.x];
    float3 localNormal = normalize(cross(
        tri.vertexOffsets[1] - tri.vertexOffsets[0],
        tri.vertexOffsets[2] - tri.vertexOffsets[0]));
    float3 worldNormal = ToWorldNormal(tri.instData, localNormal);

    for (uint i = 0; i < 3; ++i)
    {
        // 頂点を変換
        float4 localPos = float4(tri.vertexOffsets[i], 1.0f);
        float4 worldPos = mul(localPos, tri.instData.worldMatrix);
        vertices[i].position = mul(worldPos, gCamera.viewProj);
        // 色をセット
        vertices[i].baseColor = tri.instData.baseColor;
        vertices[i].uv = (i == 0 ? float2(0.5f, 0.0f) : (i == 1 ? float2(0.0f, 1.0f) : float2(1.0f, 1.0f))) * tri.instData.uvScale + tri.instData.uvOffset;
        vertices[i].textureIndex = tri.instData.textureIndex;
        vertices[i].normalTextureIndex = tri.instData.normalTextureIndex;
        vertices[i].normalScale = tri.instData.normalScale;
        vertices[i].worldPosition = worldPos.xyz;
        vertices[i].tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
        vertices[i].pbrTextureIndices = uint2(
            tri.instData.metallicRoughnessTextureIndex,
            tri.instData.occlusionTextureIndex);
        vertices[i].pbrFactors = float3(
            tri.instData.metallic,
            tri.instData.roughness,
            tri.instData.occlusionStrength);
        vertices[i].normal.xyz = worldNormal;
        vertices[i].normal.w = GetUnlitFlag(tri.instData);
    }

    triangles[0] = uint3(0, 1, 2);
    
}

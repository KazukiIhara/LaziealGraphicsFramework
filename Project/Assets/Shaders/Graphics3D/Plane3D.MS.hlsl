#include "Graphics3D.hlsli"

struct PlaneData
{
    InstanceData3D instData;
    float3 vertexOffsets[4];
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<PlaneData> gPlane : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[4],
    out indices uint3 triangles[2])
{
    SetMeshOutputCounts(4, 2);

    PlaneData plane = gPlane[DTid.x];
    float3 worldNormal = ToWorldNormal(plane.instData, float3(0.0f, 0.0f, 1.0f));

    for (uint i = 0; i < 4; ++i)
    {
        float4 localPos = float4(plane.vertexOffsets[i], 1.0f);
        float4 worldPos = mul(localPos, plane.instData.worldMatrix);
        vertices[i].position = mul(worldPos, gCamera.viewProj);
        vertices[i].baseColor = plane.instData.baseColor;
        vertices[i].uv = float2(i % 2, 1 - i / 2) * plane.instData.uvScale + plane.instData.uvOffset;
        vertices[i].textureIndex = plane.instData.textureIndex;
        vertices[i].normalTextureIndex = plane.instData.normalTextureIndex;
        vertices[i].normalScale = plane.instData.normalScale;
        vertices[i].worldPosition = worldPos.xyz;
        vertices[i].tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
        vertices[i].normal.xyz = worldNormal;
        vertices[i].normal.w = GetUnlitFlag(plane.instData);
    }

    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(1, 3, 2);
}

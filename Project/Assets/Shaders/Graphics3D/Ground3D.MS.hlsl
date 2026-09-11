#include "Graphics3D.hlsli"

struct GroundData
{
    InstanceData3D instData;
    float3 vertexOffsets[4];
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<GroundData> gGround : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[4],
    out indices uint3 triangles[2])
{
    SetMeshOutputCounts(4, 2);

    GroundData ground = gGround[DTid.x];
    float3 worldNormal = ToWorldNormal(ground.instData, float3(0.0f, 1.0f, 0.0f));

    for (uint i = 0; i < 4; ++i)
    {
        float4 localPos = float4(ground.vertexOffsets[i], 1.0f);
        float4 worldPos = mul(localPos, ground.instData.worldMatrix);
        vertices[i].position = mul(worldPos, gCamera.viewProj);
        vertices[i].baseColor = ground.instData.baseColor;
        vertices[i].uv = float2(i / 2, 1 - i % 2) * ground.instData.uvScale + ground.instData.uvOffset;
        vertices[i].textureIndex = ground.instData.textureIndex;
        vertices[i].normal = worldNormal;
    }

    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(1, 3, 2);
}

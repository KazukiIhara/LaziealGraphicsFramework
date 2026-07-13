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

    for (uint i = 0; i < 4; ++i)
    {
        float4 localPos = float4(plane.vertexOffsets[i], 1.0f);
        float4 worldPos = mul(localPos, plane.instData.worldMatrix);
        vertices[i].position = mul(worldPos, gCamera.viewProj);
        vertices[i].baseColor = plane.instData.baseColor;
    }

    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(1, 3, 2);
}

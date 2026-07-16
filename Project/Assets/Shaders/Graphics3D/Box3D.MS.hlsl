#include "Graphics3D.hlsli"

struct BoxData
{
    InstanceData3D instData;
    float3 halfSize;
    float padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<BoxData> gBox : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[8],
    out indices uint3 triangles[12])
{
    SetMeshOutputCounts(8, 12);

    BoxData box = gBox[DTid.x];

    float3 hs = box.halfSize;
    float3 localPositions[8] =
    {
        float3(-hs.x, -hs.y, -hs.z),
        float3( hs.x, -hs.y, -hs.z),
        float3(-hs.x,  hs.y, -hs.z),
        float3( hs.x,  hs.y, -hs.z),
        float3(-hs.x, -hs.y,  hs.z),
        float3( hs.x, -hs.y,  hs.z),
        float3(-hs.x,  hs.y,  hs.z),
        float3( hs.x,  hs.y,  hs.z)
    };

    for (uint i = 0; i < 8; ++i)
    {
        float4 worldPos = mul(float4(localPositions[i], 1.0f), box.instData.worldMatrix);
        vertices[i].position = mul(worldPos, gCamera.viewProj);
        vertices[i].baseColor = box.instData.baseColor;
    }

    triangles[0] = uint3(0, 2, 1);
    triangles[1] = uint3(1, 2, 3);

    triangles[2] = uint3(4, 5, 6);
    triangles[3] = uint3(5, 7, 6);

    triangles[4] = uint3(0, 4, 2);
    triangles[5] = uint3(4, 6, 2);

    triangles[6] = uint3(1, 3, 5);
    triangles[7] = uint3(5, 3, 7);

    triangles[8] = uint3(2, 6, 3);
    triangles[9] = uint3(3, 6, 7);

    triangles[10] = uint3(0, 1, 4);
    triangles[11] = uint3(1, 5, 4);
}

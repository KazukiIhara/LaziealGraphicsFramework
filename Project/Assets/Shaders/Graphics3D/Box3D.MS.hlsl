#include "Graphics3D.hlsli"

struct BoxData { InstanceData3D instData; float3 halfSize; float padding; };
ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<BoxData> gBox : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[24], out indices uint3 triangles[12])
{
    SetMeshOutputCounts(24, 12);
    BoxData box = gBox[id.x];
    float3 corners[8] = {
        float3(-1,-1,-1), float3(1,-1,-1), float3(-1,1,-1), float3(1,1,-1),
        float3(-1,-1,1), float3(1,-1,1), float3(-1,1,1), float3(1,1,1)
    };
    uint4 faces[6] = {
        uint4(0,2,1,3), uint4(4,5,6,7), uint4(0,4,2,6),
        uint4(1,3,5,7), uint4(2,6,3,7), uint4(0,1,4,5)
    };
    float3 normals[6] = {
        float3(0,0,-1), float3(0,0,1), float3(-1,0,0),
        float3(1,0,0), float3(0,1,0), float3(0,-1,0)
    };
    for (uint face = 0; face < 6; ++face)
    {
        uint start = face * 4;
        for (uint i = 0; i < 4; ++i)
        {
            vertices[start + i] = MakeVertex(box.instData, gCamera,
                corners[faces[face][i]] * box.halfSize, normals[face],
                float2(i / 2, 1 - i % 2));
        }
        triangles[face * 2] = uint3(start, start + 1, start + 2);
        triangles[face * 2 + 1] = uint3(start + 1, start + 3, start + 2);
    }
}

#include "Graphics3D.hlsli"

static const uint kSegments = 32;
static const uint kRing = kSegments + 1;
static const uint kBottomCenter = kRing * 4;
static const uint kTopCenter = kBottomCenter + 1;
struct CylinderData { InstanceData3D instData; float radius; float height; float2 padding; };
ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<CylinderData> gCylinder : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[kRing * 4 + 2],
    out indices uint3 triangles[kSegments * 4])
{
    SetMeshOutputCounts(kRing * 4 + 2, kSegments * 4);
    CylinderData cylinder = gCylinder[id.x];
    float h = cylinder.height * 0.5f;
    for (uint i = 0; i <= kSegments; ++i)
    {
        float u = (float)i / kSegments;
        float angle = u * 6.28318530718f;
        float3 n = float3(cos(angle), 0, sin(angle));
        float3 bottom = float3(n.x * cylinder.radius, -h, n.z * cylinder.radius);
        float3 top = float3(bottom.x, h, bottom.z);
        vertices[i] = MakeVertex(cylinder.instData, gCamera, bottom, n, float2(u, 1));
        vertices[kRing+i] = MakeVertex(cylinder.instData, gCamera, top, n, float2(u, 0));
        vertices[kRing*2+i] = MakeVertex(cylinder.instData, gCamera, bottom,
            float3(0,-1,0), n.xz * 0.5f + 0.5f);
        vertices[kRing*3+i] = MakeVertex(cylinder.instData, gCamera, top,
            float3(0,1,0), n.xz * 0.5f + 0.5f);
    }
    vertices[kBottomCenter] = MakeVertex(cylinder.instData, gCamera,
        float3(0,-h,0), float3(0,-1,0), float2(0.5f,0.5f));
    vertices[kTopCenter] = MakeVertex(cylinder.instData, gCamera,
        float3(0,h,0), float3(0,1,0), float2(0.5f,0.5f));
    for (uint i = 0; i < kSegments; ++i)
    {
        triangles[i*4] = uint3(i, kRing+i, i+1);
        triangles[i*4+1] = uint3(i+1, kRing+i, kRing+i+1);
        triangles[i*4+2] = uint3(kBottomCenter, kRing*2+i, kRing*2+i+1);
        triangles[i*4+3] = uint3(kTopCenter, kRing*3+i+1, kRing*3+i);
    }
}

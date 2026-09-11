#include "Graphics3D.hlsli"

static const uint kSegments = 32;
static const uint kRing = kSegments + 1;
static const uint kBottomRing = kRing + kSegments;
static const uint kBottomCenter = kBottomRing + kRing;
struct ConeData { InstanceData3D instData; float radius; float height; float2 padding; };
ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<ConeData> gCone : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[kBottomCenter + 1],
    out indices uint3 triangles[kSegments * 2])
{
    SetMeshOutputCounts(kBottomCenter + 1, kSegments * 2);
    ConeData cone = gCone[id.x];
    float h = cone.height * 0.5f;
    for (uint i = 0; i <= kSegments; ++i)
    {
        float u = (float)i / kSegments;
        float angle = u * 6.28318530718f;
        float2 radial = float2(cos(angle), sin(angle));
        float3 position = float3(radial.x * cone.radius, -h, radial.y * cone.radius);
        float3 normal = normalize(float3(radial.x * cone.height, cone.radius, radial.y * cone.height));
        vertices[i] = MakeVertex(cone.instData, gCamera, position, normal, float2(u,1));
        vertices[kBottomRing+i] = MakeVertex(cone.instData, gCamera, position,
            float3(0,-1,0), radial * 0.5f + 0.5f);
        if (i < kSegments)
        {
            float tipU = ((float)i + 0.5f) / kSegments;
            float tipAngle = tipU * 6.28318530718f;
            float3 tipNormal = normalize(float3(cos(tipAngle)*cone.height, cone.radius, sin(tipAngle)*cone.height));
            vertices[kRing+i] = MakeVertex(cone.instData, gCamera,
                float3(0,h,0), tipNormal, float2(tipU,0));
        }
    }
    vertices[kBottomCenter] = MakeVertex(cone.instData, gCamera,
        float3(0,-h,0), float3(0,-1,0), float2(0.5f,0.5f));
    for (uint i = 0; i < kSegments; ++i)
    {
        triangles[i*2] = uint3(i, kRing+i, i+1);
        triangles[i*2+1] = uint3(kBottomCenter, kBottomRing+i, kBottomRing+i+1);
    }
}

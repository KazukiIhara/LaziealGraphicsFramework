#include "Graphics3D.hlsli"

static const uint kSegments = 32;
static const uint kBottomCenterIndex = kSegments;
static const uint kTipIndex = kSegments + 1;
static const uint kVertexCount = kSegments + 2;
static const uint kTriangleCount = kSegments * 2;
static const float kPI = 3.14159265358979323846f;

struct ConeData
{
    InstanceData3D instData;
    float radius;
    float height;
    float2 padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<ConeData> gCone : register(t0);

float4 ToClipPosition(ConeData cone, float3 localPosition)
{
    float4 worldPos = mul(float4(localPosition, 1.0f), cone.instData.worldMatrix);
    return mul(worldPos, gCamera.viewProj);
}

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[kVertexCount],
    out indices uint3 triangles[kTriangleCount])
{
    SetMeshOutputCounts(kVertexCount, kTriangleCount);

    ConeData cone = gCone[DTid.x];

    float halfHeight = cone.height * 0.5f;

    for (uint i = 0; i < kSegments; ++i)
    {
        float theta = ((float)i / (float)kSegments) * kPI * 2.0f;
        float x = cos(theta) * cone.radius;
        float z = sin(theta) * cone.radius;

        vertices[i].position = ToClipPosition(cone, float3(x, -halfHeight, z));
        vertices[i].baseColor = cone.instData.baseColor;
        vertices[i].normal = ToWorldNormal(cone.instData, normalize(float3(x, cone.radius / cone.height, z)));
    }

    vertices[kBottomCenterIndex].position = ToClipPosition(cone, float3(0.0f, -halfHeight, 0.0f));
    vertices[kBottomCenterIndex].baseColor = cone.instData.baseColor;
    vertices[kBottomCenterIndex].normal = ToWorldNormal(cone.instData, float3(0.0f, -1.0f, 0.0f));

    vertices[kTipIndex].position = ToClipPosition(cone, float3(0.0f, halfHeight, 0.0f));
    vertices[kTipIndex].baseColor = cone.instData.baseColor;
    vertices[kTipIndex].normal = ToWorldNormal(cone.instData, float3(0.0f, 1.0f, 0.0f));

    uint triIndex = 0;

    for (uint i = 0; i < kSegments; ++i)
    {
        uint next = (i + 1) % kSegments;

        triangles[triIndex++] = uint3(i, kTipIndex, next);
        triangles[triIndex++] = uint3(kBottomCenterIndex, i, next);
    }
}

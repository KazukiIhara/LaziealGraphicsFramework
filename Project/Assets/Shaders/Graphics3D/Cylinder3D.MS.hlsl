#include "Graphics3D.hlsli"

static const uint kSegments = 32;
static const uint kVertexCount = kSegments * 2 + 2;
static const uint kBottomCenterIndex = kSegments * 2;
static const uint kTopCenterIndex = kSegments * 2 + 1;
static const uint kTriangleCount = kSegments * 4;
static const float kPI = 3.14159265358979323846f;

struct CylinderData
{
    InstanceData3D instData;
    float radius;
    float height;
    float2 padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<CylinderData> gCylinder : register(t0);

float4 ToClipPosition(CylinderData cylinder, float3 localPosition)
{
    float4 worldPos = mul(float4(localPosition, 1.0f), cylinder.instData.worldMatrix);
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

    CylinderData cylinder = gCylinder[DTid.x];

    float halfHeight = cylinder.height * 0.5f;

    for (uint i = 0; i < kSegments; ++i)
    {
        float theta = ((float)i / (float)kSegments) * kPI * 2.0f;
        float x = cos(theta) * cylinder.radius;
        float z = sin(theta) * cylinder.radius;

        vertices[i].position = ToClipPosition(cylinder, float3(x, -halfHeight, z));
        vertices[i].baseColor = cylinder.instData.baseColor;
        vertices[i].normal = ToWorldNormal(cylinder.instData, normalize(float3(x, 0.0f, z)));

        vertices[kSegments + i].position = ToClipPosition(cylinder, float3(x, halfHeight, z));
        vertices[kSegments + i].baseColor = cylinder.instData.baseColor;
        vertices[kSegments + i].normal = ToWorldNormal(cylinder.instData, normalize(float3(x, 0.0f, z)));
    }

    vertices[kBottomCenterIndex].position = ToClipPosition(cylinder, float3(0.0f, -halfHeight, 0.0f));
    vertices[kBottomCenterIndex].baseColor = cylinder.instData.baseColor;
    vertices[kBottomCenterIndex].normal = ToWorldNormal(cylinder.instData, float3(0.0f, -1.0f, 0.0f));

    vertices[kTopCenterIndex].position = ToClipPosition(cylinder, float3(0.0f, halfHeight, 0.0f));
    vertices[kTopCenterIndex].baseColor = cylinder.instData.baseColor;
    vertices[kTopCenterIndex].normal = ToWorldNormal(cylinder.instData, float3(0.0f, 1.0f, 0.0f));

    uint triIndex = 0;

    for (uint i = 0; i < kSegments; ++i)
    {
        uint next = (i + 1) % kSegments;

        uint bottom0 = i;
        uint bottom1 = next;
        uint top0 = kSegments + i;
        uint top1 = kSegments + next;

        triangles[triIndex++] = uint3(bottom0, top0, bottom1);
        triangles[triIndex++] = uint3(bottom1, top0, top1);

        triangles[triIndex++] = uint3(kBottomCenterIndex, bottom0, bottom1);
        triangles[triIndex++] = uint3(kTopCenterIndex, top1, top0);
    }
}

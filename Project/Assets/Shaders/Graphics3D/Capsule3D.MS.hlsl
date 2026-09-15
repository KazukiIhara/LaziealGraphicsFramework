#include "Graphics3D.hlsli"

static const uint kLatitudeSegments = 16;
static const uint kLongitudeSegments = 32;
static const uint kMeshletCount = 4;
static const uint kMeshletLatitudeSegments = kLatitudeSegments / kMeshletCount;
static const uint kMeshletVertexCount = (kMeshletLatitudeSegments + 1) * (kLongitudeSegments + 1);
static const uint kMeshletTriangleCount = kMeshletLatitudeSegments * kLongitudeSegments * 2;
static const float kPI = 3.14159265358979323846f;

struct CapsuleData
{
    InstanceData3D instData;
    float radius;
    float height;
    float2 padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<CapsuleData> gCapsule : register(t0);

uint GetVertexIndex(uint lat, uint lon)
{
    return lat * (kLongitudeSegments + 1) + lon;
}

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[kMeshletVertexCount],
    out indices uint3 triangles[kMeshletTriangleCount])
{
    SetMeshOutputCounts(kMeshletVertexCount, kMeshletTriangleCount);

    uint capsuleIndex = DTid.x / kMeshletCount;
    uint meshletIndex = DTid.x % kMeshletCount;
    uint latitudeOffset = meshletIndex * kMeshletLatitudeSegments;

    CapsuleData capsule = gCapsule[capsuleIndex];

    float radius = min(capsule.radius, capsule.height * 0.5f);
    float halfCylinderHeight = max(capsule.height * 0.5f - radius, 0.0f);

    for (uint lat = 0; lat <= kMeshletLatitudeSegments; ++lat)
    {
        float v = (float)(latitudeOffset + lat) / (float)kLatitudeSegments;
        float phi = v * kPI;

        for (uint lon = 0; lon <= kLongitudeSegments; ++lon)
        {
            float u = (float)lon / (float)kLongitudeSegments;
            float theta = u * kPI * 2.0f;

            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            float yOffset = cosPhi >= 0.0f ? halfCylinderHeight : -halfCylinderHeight;

            float3 localPos = float3(
                sinPhi * cos(theta) * radius,
                cosPhi * radius + yOffset,
                sinPhi * sin(theta) * radius);
            float3 localNormal = normalize(float3(
                sinPhi * cos(theta),
                cosPhi,
                sinPhi * sin(theta)));

            float4 worldPos = mul(float4(localPos, 1.0f), capsule.instData.worldMatrix);

            uint vertexIndex = GetVertexIndex(lat, lon);
            vertices[vertexIndex].position = mul(worldPos, gCamera.viewProj);
            vertices[vertexIndex].baseColor = capsule.instData.baseColor;
            vertices[vertexIndex].uv = float2(u, v) * capsule.instData.uvScale + capsule.instData.uvOffset;
            vertices[vertexIndex].textureIndex = capsule.instData.textureIndex;
            vertices[vertexIndex].normalTextureIndex = capsule.instData.normalTextureIndex;
            vertices[vertexIndex].normalScale = capsule.instData.normalScale;
            vertices[vertexIndex].worldPosition = worldPos.xyz;
            vertices[vertexIndex].tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
            vertices[vertexIndex].normal = ToWorldNormal(capsule.instData, localNormal);
        }
    }

    uint triIndex = 0;

    for (uint lat = 0; lat < kMeshletLatitudeSegments; ++lat)
    {
        for (uint lon = 0; lon < kLongitudeSegments; ++lon)
        {
            uint v00 = GetVertexIndex(lat, lon);
            uint v01 = GetVertexIndex(lat, lon + 1);
            uint v10 = GetVertexIndex(lat + 1, lon);
            uint v11 = GetVertexIndex(lat + 1, lon + 1);

            triangles[triIndex++] = uint3(v00, v01, v10);
            triangles[triIndex++] = uint3(v01, v11, v10);
        }
    }
}

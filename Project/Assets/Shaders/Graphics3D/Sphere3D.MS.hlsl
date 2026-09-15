#include "Graphics3D.hlsli"

static const uint kLatitudeSegments = 16;
static const uint kLongitudeSegments = 32;
static const uint kMeshletCount = 4;
static const uint kMeshletLatitudeSegments = kLatitudeSegments / kMeshletCount;
static const uint kMeshletVertexCount = (kMeshletLatitudeSegments + 1) * (kLongitudeSegments + 1);
static const uint kMeshletTriangleCount = kMeshletLatitudeSegments * kLongitudeSegments * 2;
static const float kPI = 3.14159265358979323846f;

struct SphereData
{
    InstanceData3D instData;
    float radius;
    float3 padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<SphereData> gSphere : register(t0);

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

    uint sphereIndex = DTid.x / kMeshletCount;
    uint meshletIndex = DTid.x % kMeshletCount;
    uint latitudeOffset = meshletIndex * kMeshletLatitudeSegments;

    SphereData sphere = gSphere[sphereIndex];

    for (uint lat = 0; lat <= kMeshletLatitudeSegments; ++lat)
    {
        float v = (float)(latitudeOffset + lat) / (float)kLatitudeSegments;
        float phi = v * kPI;

        for (uint lon = 0; lon <= kLongitudeSegments; ++lon)
        {
            float u = (float)lon / (float)kLongitudeSegments;
            float theta = u * kPI * 2.0f;

            float sinPhi = sin(phi);
            float3 localPos = float3(
                sinPhi * cos(theta),
                cos(phi),
                sinPhi * sin(theta)) * sphere.radius;
            float3 localNormal = normalize(localPos);

            float4 worldPos = mul(float4(localPos, 1.0f), sphere.instData.worldMatrix);

            uint vertexIndex = GetVertexIndex(lat, lon);
            vertices[vertexIndex].position = mul(worldPos, gCamera.viewProj);
            vertices[vertexIndex].baseColor = sphere.instData.baseColor;
            vertices[vertexIndex].uv = float2(u, v) * sphere.instData.uvScale + sphere.instData.uvOffset;
            vertices[vertexIndex].textureIndex = sphere.instData.textureIndex;
            vertices[vertexIndex].normalTextureIndex = sphere.instData.normalTextureIndex;
            vertices[vertexIndex].normalScale = sphere.instData.normalScale;
            vertices[vertexIndex].worldPosition = worldPos.xyz;
            vertices[vertexIndex].tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
            vertices[vertexIndex].pbrTextureIndices = uint2(
                sphere.instData.metallicRoughnessTextureIndex,
                sphere.instData.occlusionTextureIndex);
            vertices[vertexIndex].pbrFactors = float3(
                sphere.instData.metallic,
                sphere.instData.roughness,
                sphere.instData.occlusionStrength);
            vertices[vertexIndex].normal.xyz = ToWorldNormal(sphere.instData, localNormal);
            vertices[vertexIndex].normal.w = GetUnlitFlag(sphere.instData);
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

#include "Graphics3D.hlsli"

static const uint kLatitudeSegments = 16;
static const uint kLongitudeSegments = 32;
static const uint kMeshletCount = 4;
static const uint kMeshletLatitudeSegments = kLatitudeSegments / kMeshletCount;
static const uint kMeshletVertexCount = (kMeshletLatitudeSegments + 1) * (kLongitudeSegments + 1);
static const uint kMeshletTriangleCount = kMeshletLatitudeSegments * kLongitudeSegments * 2;
static const float kPI = 3.14159265358979323846f;

struct SphericalCapData
{
    InstanceData3D instData;
    float baseRadius;
    float height;
    float2 padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<SphericalCapData> gSphericalCap : register(t0);

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

    uint capIndex = DTid.x / kMeshletCount;
    uint meshletIndex = DTid.x % kMeshletCount;
    uint latitudeOffset = meshletIndex * kMeshletLatitudeSegments;

    SphericalCapData cap = gSphericalCap[capIndex];
    float baseRadius = max(abs(cap.baseRadius), 0.0001f);
    float height = max(abs(cap.height), 0.0001f);
    float sphereRadius = (baseRadius * baseRadius + height * height) / (2.0f * height);
    float capAngle = 2.0f * atan(height / baseRadius);
    float3 sphereCenter = float3(0.0f, sphereRadius, 0.0f);

    for (uint lat = 0; lat <= kMeshletLatitudeSegments; ++lat)
    {
        float v = (float)(latitudeOffset + lat) / (float)kLatitudeSegments;
        float phi = v * capAngle;

        for (uint lon = 0; lon <= kLongitudeSegments; ++lon)
        {
            float u = (float)lon / (float)kLongitudeSegments;
            float theta = u * kPI * 2.0f;
            float sinPhi = sin(phi);
            float3 localPos = float3(
                sphereRadius * sinPhi * cos(theta),
                sphereRadius * (1.0f - cos(phi)),
                sphereRadius * sinPhi * sin(theta));
            float3 localNormal = normalize(sphereCenter - localPos);
            float4 worldPos = mul(float4(localPos, 1.0f), cap.instData.worldMatrix);

            uint vertexIndex = GetVertexIndex(lat, lon);
            vertices[vertexIndex].position = mul(worldPos, gCamera.viewProj);
            vertices[vertexIndex].baseColor = cap.instData.baseColor;
            vertices[vertexIndex].uv = float2(u, v) * cap.instData.uvScale + cap.instData.uvOffset;
            vertices[vertexIndex].textureIndex = cap.instData.textureIndex;
            vertices[vertexIndex].normalTextureIndex = cap.instData.normalTextureIndex;
            vertices[vertexIndex].normalScale = cap.instData.normalScale;
            vertices[vertexIndex].worldPosition = worldPos.xyz;
            vertices[vertexIndex].tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
            vertices[vertexIndex].pbrTextureIndices = uint2(
                cap.instData.metallicRoughnessTextureIndex,
                cap.instData.occlusionTextureIndex);
            vertices[vertexIndex].pbrFactors = float3(
                cap.instData.metallic,
                cap.instData.roughness,
                cap.instData.occlusionStrength);
            vertices[vertexIndex].normal.xyz = ToWorldNormal(cap.instData, localNormal);
            vertices[vertexIndex].normal.w = GetUnlitFlag(cap.instData);
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

			// 頂点順は球面の内側が表面になる向きにそろえる。
			triangles[triIndex++] = uint3(v00, v01, v10);
			triangles[triIndex++] = uint3(v01, v11, v10);
        }
    }
}

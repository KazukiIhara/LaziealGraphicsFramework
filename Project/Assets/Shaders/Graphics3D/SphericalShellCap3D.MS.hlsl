#include "Graphics3D.hlsli"

static const uint kLatitudeSegments = 16;
static const uint kLongitudeSegments = 32;
static const uint kSurfaceMeshletCount = 4;
static const uint kMeshletLatitudeSegments = kLatitudeSegments / kSurfaceMeshletCount;
static const uint kMeshletVertexCount = (kMeshletLatitudeSegments + 1) * (kLongitudeSegments + 1);
static const uint kMeshletTriangleCount = kMeshletLatitudeSegments * kLongitudeSegments * 2;
static const uint kRimVertexCount = (kLongitudeSegments + 1) * 2;
static const uint kRimTriangleCount = kLongitudeSegments * 2;
static const uint kMeshletCount = kSurfaceMeshletCount * 2 + 1;
static const float kPI = 3.14159265358979323846f;

struct SphericalShellCapData
{
    InstanceData3D instData;
    float outerRadius;
    float innerRadius;
    float height;
    float padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<SphericalShellCapData> gSphericalShellCap : register(t0);

uint GetSurfaceVertexIndex(uint lat, uint lon)
{
    return lat * (kLongitudeSegments + 1) + lon;
}

void WriteVertex(
    SphericalShellCapData cap,
    float3 localPosition,
    float3 localNormal,
    float2 uv,
    uint vertexIndex,
    out vertices MSOutput3D vertices[kMeshletVertexCount])
{
    float4 worldPosition = mul(float4(localPosition, 1.0f), cap.instData.worldMatrix);
    vertices[vertexIndex].position = mul(worldPosition, gCamera.viewProj);
    vertices[vertexIndex].baseColor = cap.instData.baseColor;
    vertices[vertexIndex].uv = uv * cap.instData.uvScale + cap.instData.uvOffset;
    vertices[vertexIndex].textureIndex = cap.instData.textureIndex;
    vertices[vertexIndex].normalTextureIndex = cap.instData.normalTextureIndex;
    vertices[vertexIndex].normalScale = cap.instData.normalScale;
    vertices[vertexIndex].worldPosition = worldPosition.xyz;
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

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[kMeshletVertexCount],
    out indices uint3 triangles[kMeshletTriangleCount])
{
    uint capIndex = DTid.x / kMeshletCount;
    uint localMeshletIndex = DTid.x % kMeshletCount;
    SphericalShellCapData cap = gSphericalShellCap[capIndex];

    float outerRadius = max(abs(cap.outerRadius), 0.0001f);
    float innerRadius = clamp(abs(cap.innerRadius), 0.0001f, outerRadius - 0.0001f);
    float thickness = outerRadius - innerRadius;
    float height = clamp(abs(cap.height), thickness + 0.0001f, outerRadius);
    float3 sphereCenter = float3(0.0f, outerRadius, 0.0f);
	bool isSurfaceMeshlet = localMeshletIndex < kSurfaceMeshletCount * 2;
	SetMeshOutputCounts(
		isSurfaceMeshlet ? kMeshletVertexCount : kRimVertexCount,
		isSurfaceMeshlet ? kMeshletTriangleCount : kRimTriangleCount);

    if (isSurfaceMeshlet)
    {
        bool isInnerSurface = localMeshletIndex < kSurfaceMeshletCount;
        uint surfaceMeshletIndex = isInnerSurface
            ? localMeshletIndex
            : localMeshletIndex - kSurfaceMeshletCount;
        uint latitudeOffset = surfaceMeshletIndex * kMeshletLatitudeSegments;
        float radius = isInnerSurface ? innerRadius : outerRadius;
        float capAngle = acos(clamp((outerRadius - height) / radius, -1.0f, 1.0f));

        for (uint lat = 0; lat <= kMeshletLatitudeSegments; ++lat)
        {
            float v = (float)(latitudeOffset + lat) / (float)kLatitudeSegments;
            float phi = v * capAngle;
            for (uint lon = 0; lon <= kLongitudeSegments; ++lon)
            {
                float u = (float)lon / (float)kLongitudeSegments;
                float theta = u * kPI * 2.0f;
                float sinPhi = sin(phi);
                float3 localPosition = float3(
                    radius * sinPhi * cos(theta),
                    outerRadius - radius * cos(phi),
                    radius * sinPhi * sin(theta));
                float3 radialNormal = normalize(localPosition - sphereCenter);
                float3 localNormal = isInnerSurface ? -radialNormal : radialNormal;
                WriteVertex(
                    cap,
                    localPosition,
                    localNormal,
                    float2(u, v),
                    GetSurfaceVertexIndex(lat, lon),
                    vertices);
            }
        }

        uint triangleIndex = 0;
        for (uint lat = 0; lat < kMeshletLatitudeSegments; ++lat)
        {
            for (uint lon = 0; lon < kLongitudeSegments; ++lon)
            {
                uint v00 = GetSurfaceVertexIndex(lat, lon);
                uint v01 = GetSurfaceVertexIndex(lat, lon + 1);
                uint v10 = GetSurfaceVertexIndex(lat + 1, lon);
                uint v11 = GetSurfaceVertexIndex(lat + 1, lon + 1);
                if (isInnerSurface)
                {
                    triangles[triangleIndex++] = uint3(v00, v01, v10);
                    triangles[triangleIndex++] = uint3(v01, v11, v10);
                }
                else
                {
                    triangles[triangleIndex++] = uint3(v00, v10, v01);
                    triangles[triangleIndex++] = uint3(v01, v10, v11);
                }
            }
        }
        return;
    }

    float planeOffset = height - outerRadius;
    float outerOpeningRadius = sqrt(max(
        outerRadius * outerRadius - planeOffset * planeOffset,
        0.0f));
    float innerOpeningRadius = sqrt(max(
        innerRadius * innerRadius - planeOffset * planeOffset,
        0.0f));

    for (uint ring = 0; ring < 2; ++ring)
    {
        float radius = ring == 0 ? innerOpeningRadius : outerOpeningRadius;
        for (uint lon = 0; lon <= kLongitudeSegments; ++lon)
        {
            float u = (float)lon / (float)kLongitudeSegments;
            float theta = u * kPI * 2.0f;
            float3 localPosition = float3(
                radius * cos(theta),
                height,
                radius * sin(theta));
            uint vertexIndex = ring * (kLongitudeSegments + 1) + lon;
            WriteVertex(
                cap,
                localPosition,
                float3(0.0f, 1.0f, 0.0f),
                float2(u, (float)ring),
                vertexIndex,
                vertices);
        }
    }

    uint triangleIndex = 0;
    for (uint lon = 0; lon < kLongitudeSegments; ++lon)
    {
        uint inner0 = lon;
        uint inner1 = lon + 1;
        uint outer0 = kLongitudeSegments + 1 + lon;
        uint outer1 = outer0 + 1;
        triangles[triangleIndex++] = uint3(inner0, inner1, outer0);
        triangles[triangleIndex++] = uint3(inner1, outer1, outer0);
    }
}

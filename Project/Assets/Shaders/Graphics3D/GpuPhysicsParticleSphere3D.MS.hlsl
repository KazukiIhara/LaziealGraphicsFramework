#include "Graphics3D.hlsli"
#include "../Compute/GpuPhysicsParticle.hlsli"

static const uint kLatitudeSegments = 16u;
static const uint kLongitudeSegments = 32u;
static const uint kMeshletCount = 4u;
static const uint kMeshletLatitudeSegments = kLatitudeSegments / kMeshletCount;
static const uint kMeshletVertexCount = (kMeshletLatitudeSegments + 1u) * (kLongitudeSegments + 1u);
static const uint kMeshletTriangleCount = kMeshletLatitudeSegments * kLongitudeSegments * 2u;
static const float kPI = 3.14159265358979323846f;

cbuffer GpuPhysicsParticleSphereConstants : register(b1)
{
	uint gRenderGroupIndex;
	uint gParticleOffset;
	uint gParticleCount;
	uint gPadding;
	float4 gColor;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);

uint GetVertexIndex(uint latitude, uint longitude)
{
	return latitude * (kLongitudeSegments + 1u) + longitude;
}

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
	uint3 id : SV_DispatchThreadID,
	out vertices MSOutput3D vertices[kMeshletVertexCount],
	out indices uint3 triangles[kMeshletTriangleCount])
{
	const uint localParticleIndex = id.x / kMeshletCount;
	const uint particleIndex = gParticleOffset + localParticleIndex;
	GpuPhysicsParticleData particle = (GpuPhysicsParticleData)0;
	if (localParticleIndex < gParticleCount)
	{
		particle = gParticles[particleIndex];
	}
	const bool isVisible = localParticleIndex < gParticleCount &&
		particle.alive != 0u && particle.enabled != 0u &&
		particle.renderGroupIndex == gRenderGroupIndex;
	SetMeshOutputCounts(
		isVisible ? kMeshletVertexCount : 0u,
		isVisible ? kMeshletTriangleCount : 0u);
	if (!isVisible)
	{
		return;
	}

	const uint meshletIndex = id.x % kMeshletCount;
	const uint latitudeOffset = meshletIndex * kMeshletLatitudeSegments;
	for (uint latitude = 0u; latitude <= kMeshletLatitudeSegments; ++latitude)
	{
		const float v = float(latitudeOffset + latitude) / float(kLatitudeSegments);
		const float phi = v * kPI;
		for (uint longitude = 0u; longitude <= kLongitudeSegments; ++longitude)
		{
			const float u = float(longitude) / float(kLongitudeSegments);
			const float theta = u * kPI * 2.0f;
			const float sinPhi = sin(phi);
			const float3 normal = float3(
				sinPhi * cos(theta),
				cos(phi),
				sinPhi * sin(theta));
			const float3 worldPosition = particle.position + normal * particle.radius;
			const uint vertexIndex = GetVertexIndex(latitude, longitude);
			vertices[vertexIndex].position = mul(float4(worldPosition, 1.0f), gCamera.viewProj);
			vertices[vertexIndex].baseColor = gColor;
			vertices[vertexIndex].uv = float2(u, v);
			vertices[vertexIndex].textureIndex = 0xffffffffu;
			vertices[vertexIndex].normalTextureIndex = 0xffffffffu;
			vertices[vertexIndex].normalScale = 1.0f;
			vertices[vertexIndex].worldPosition = worldPosition;
			vertices[vertexIndex].tangent = 0.0f;
			vertices[vertexIndex].pbrTextureIndices = 0xffffffffu;
			vertices[vertexIndex].pbrFactors = float3(0.0f, 1.0f, 1.0f);
			vertices[vertexIndex].normal = float4(normal, 0.0f);
		}
	}

	uint triangleIndex = 0u;
	for (uint latitude = 0u; latitude < kMeshletLatitudeSegments; ++latitude)
	{
		for (uint longitude = 0u; longitude < kLongitudeSegments; ++longitude)
		{
			const uint v00 = GetVertexIndex(latitude, longitude);
			const uint v01 = GetVertexIndex(latitude, longitude + 1u);
			const uint v10 = GetVertexIndex(latitude + 1u, longitude);
			const uint v11 = GetVertexIndex(latitude + 1u, longitude + 1u);
			triangles[triangleIndex++] = uint3(v00, v01, v10);
			triangles[triangleIndex++] = uint3(v01, v11, v10);
		}
	}
}

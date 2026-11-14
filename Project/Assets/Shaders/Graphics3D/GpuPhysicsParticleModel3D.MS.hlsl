#include "Graphics3D.hlsli"
#include "Model3D.hlsli"
#include "../Compute/GpuPhysicsParticle.hlsli"

struct ModelVertex
{
	float3 position;
	float3 normal;
	float2 uv;
	float4 tangent;
};

cbuffer GpuPhysicsParticleModelConstants : register(b1)
{
	float4 gMeshBaseColor;
	uint gVertexBufferIndex;
	uint gMeshletBufferIndex;
	uint gMeshletVertexIndexBufferIndex;
	uint gMeshletPrimitiveBufferIndex;
	uint gBaseColorTextureIndex;
	uint gNormalTextureIndex;
	uint gMetallicRoughnessTextureIndex;
	uint gOcclusionTextureIndex;
	float gNormalScale;
	float gMetallicFactor;
	float gRoughnessFactor;
	float gOcclusionStrength;
	uint gRenderGroupIndex;
	uint gDrawMaximumParticleCount;
	uint gMeshletCount;
	uint gBaseWorkIndex;
	float3 gVisualScale;
	float gPadding;
	float4 gColor;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);

static const uint kMaxMeshletVertices = 128u;
static const uint kMaxMeshletPrimitives = 128u;
groupshared GpuPhysicsParticleData gParticle;
groupshared Meshlet gMeshlet;
groupshared uint gIsVisible;

[outputtopology("triangle")]
[numthreads(kMaxMeshletVertices, 1, 1)]
void main(
	uint3 groupId : SV_GroupID,
	uint3 groupThreadId : SV_GroupThreadID,
	out vertices MSOutput3D outputVertices[kMaxMeshletVertices],
	out indices uint3 outputTriangles[kMaxMeshletPrimitives])
{
	const uint workIndex = gBaseWorkIndex + groupId.x;
	const uint particleIndex = workIndex / gMeshletCount;
	const uint meshletIndex = workIndex % gMeshletCount;
	if (groupThreadId.x == 0u)
	{
		gParticle = (GpuPhysicsParticleData)0;
		if (particleIndex < gDrawMaximumParticleCount)
		{
			gParticle = gParticles[particleIndex];
		}
		gIsVisible = particleIndex < gDrawMaximumParticleCount &&
			gParticle.alive != 0u && gParticle.enabled != 0u &&
			gParticle.renderGroupIndex == gRenderGroupIndex;
		StructuredBuffer<Meshlet> meshletBuffer = ResourceDescriptorHeap[gMeshletBufferIndex];
		gMeshlet = (Meshlet)0;
		if (gIsVisible != 0u)
		{
			gMeshlet = meshletBuffer[meshletIndex];
		}
	}
	GroupMemoryBarrierWithGroupSync();
	SetMeshOutputCounts(gMeshlet.vertexCount, gMeshlet.primitiveCount);
	if (gIsVisible == 0u)
	{
		return;
	}

	StructuredBuffer<ModelVertex> vertexBuffer = ResourceDescriptorHeap[gVertexBufferIndex];
	StructuredBuffer<uint> meshletVertexIndexBuffer =
		ResourceDescriptorHeap[gMeshletVertexIndexBufferIndex];
	StructuredBuffer<uint> meshletPrimitiveBuffer =
		ResourceDescriptorHeap[gMeshletPrimitiveBufferIndex];
	const float3 scale = gVisualScale * gParticle.radius;
	const uint localIndex = groupThreadId.x;
	if (localIndex < gMeshlet.vertexCount)
	{
		const uint vertexIndex = meshletVertexIndexBuffer[gMeshlet.vertexOffset + localIndex];
		const ModelVertex vertex = vertexBuffer[vertexIndex];
		const float3 worldPosition = vertex.position * scale + gParticle.position;
		const float3 worldNormal = normalize(vertex.normal / scale);
		const float3 worldTangent = normalize(vertex.tangent.xyz * scale);
		outputVertices[localIndex].position = mul(float4(worldPosition, 1.0f), gCamera.viewProj);
		outputVertices[localIndex].baseColor = gMeshBaseColor * gColor;
		outputVertices[localIndex].uv = vertex.uv;
		outputVertices[localIndex].textureIndex = gBaseColorTextureIndex;
		outputVertices[localIndex].normalTextureIndex = gNormalTextureIndex;
		outputVertices[localIndex].normalScale = gNormalScale;
		outputVertices[localIndex].worldPosition = worldPosition;
		outputVertices[localIndex].tangent = float4(worldTangent, vertex.tangent.w);
		outputVertices[localIndex].pbrTextureIndices = uint2(
			gMetallicRoughnessTextureIndex, gOcclusionTextureIndex);
		outputVertices[localIndex].pbrFactors = float3(
			gMetallicFactor, gRoughnessFactor, gOcclusionStrength);
		outputVertices[localIndex].normal = float4(worldNormal, 0.0f);
	}
	if (localIndex < gMeshlet.primitiveCount)
	{
		const uint packedPrimitive =
			meshletPrimitiveBuffer[gMeshlet.primitiveOffset + localIndex];
		outputTriangles[localIndex] = uint3(
			packedPrimitive & 0x3ffu,
			(packedPrimitive >> 10u) & 0x3ffu,
			(packedPrimitive >> 20u) & 0x3ffu);
	}
}

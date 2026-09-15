#include "EnvironmentIBL.hlsli"

cbuffer EnvironmentFilterData : register(b0)
{
	uint gSourceTextureIndex;
	uint gOutputTextureIndex;
	uint gOutputSize;
	uint gSampleCount;
	float gRoughness;
	float3 gPadding;
};

SamplerState gEnvironmentSampler : register(s0);

[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
	if (dispatchId.x >= gOutputSize || dispatchId.y >= gOutputSize || dispatchId.z >= 6u)
	{
		return;
	}

	Texture2D<float4> sourceTexture = ResourceDescriptorHeap[gSourceTextureIndex];
	RWTexture2DArray<float4> outputTexture = ResourceDescriptorHeap[gOutputTextureIndex];
	const float2 uv = (float2(dispatchId.xy) + 0.5f) / float(gOutputSize);
	const float3 normal = CubemapDirection(dispatchId.z, uv);
	const float3 viewDirection = normal;

	uint sourceWidth;
	uint sourceHeight;
	uint sourceMipCount;
	sourceTexture.GetDimensions(0u, sourceWidth, sourceHeight, sourceMipCount);
	const float sourceTexelSolidAngle = 4.0f * kPi / max(float(sourceWidth * sourceHeight), 1.0f);

	float3 prefilteredColor = 0.0f;
	float totalWeight = 0.0f;
	for (uint sampleIndex = 0u; sampleIndex < gSampleCount; ++sampleIndex)
	{
		const float2 xi = Hammersley(sampleIndex, gSampleCount);
		const float3 halfwayDirection = ImportanceSampleGGX(xi, normal, max(gRoughness, 0.001f));
		const float3 lightDirection = normalize(
			2.0f * dot(viewDirection, halfwayDirection) * halfwayDirection - viewDirection);
		const float normalDotLight = saturate(dot(normal, lightDirection));
		if (normalDotLight <= 0.0f)
		{
			continue;
		}

		const float normalDotHalfway = saturate(dot(normal, halfwayDirection));
		const float halfwayDotView = saturate(dot(halfwayDirection, viewDirection));
		const float distribution = DistributionGGX(normalDotHalfway, max(gRoughness, 0.001f));
		const float pdf = max(distribution * normalDotHalfway / max(4.0f * halfwayDotView, 1.0e-6f), 1.0e-6f);
		const float sampleSolidAngle = 1.0f / (float(gSampleCount) * pdf);
		const float mipLevel = gRoughness <= 0.001f
			? 0.0f
			: max(0.5f * log2(sampleSolidAngle / sourceTexelSolidAngle), 0.0f);

		prefilteredColor += SampleEquirectangular(
			sourceTexture,
			gEnvironmentSampler,
			lightDirection,
			min(mipLevel, float(sourceMipCount - 1u))) * normalDotLight;
		totalWeight += normalDotLight;
	}

	prefilteredColor /= max(totalWeight, 1.0e-6f);
	outputTexture[dispatchId] = float4(prefilteredColor, 1.0f);
}

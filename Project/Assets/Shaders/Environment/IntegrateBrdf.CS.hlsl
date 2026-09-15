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

[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
	if (dispatchId.x >= gOutputSize || dispatchId.y >= gOutputSize)
	{
		return;
	}

	RWTexture2D<float2> outputTexture = ResourceDescriptorHeap[gOutputTextureIndex];
	const float2 uv = (float2(dispatchId.xy) + 0.5f) / float(gOutputSize);
	const float normalDotView = saturate(uv.x);
	const float roughness = saturate(uv.y);
	const float3 normal = float3(0.0f, 0.0f, 1.0f);
	const float3 viewDirection = float3(
		sqrt(saturate(1.0f - normalDotView * normalDotView)),
		0.0f,
		normalDotView);

	float scale = 0.0f;
	float bias = 0.0f;
	for (uint sampleIndex = 0u; sampleIndex < gSampleCount; ++sampleIndex)
	{
		const float2 xi = Hammersley(sampleIndex, gSampleCount);
		const float3 halfwayDirection = ImportanceSampleGGX(xi, normal, max(roughness, 0.001f));
		const float3 lightDirection = normalize(
			2.0f * dot(viewDirection, halfwayDirection) * halfwayDirection - viewDirection);
		const float normalDotLight = saturate(lightDirection.z);
		const float normalDotHalfway = saturate(halfwayDirection.z);
		const float viewDotHalfway = saturate(dot(viewDirection, halfwayDirection));
		if (normalDotLight <= 0.0f)
		{
			continue;
		}

		const float geometry = GeometrySmithIBL(normalDotView, normalDotLight, roughness);
		const float visibility =
			geometry * viewDotHalfway /
			max(normalDotHalfway * normalDotView, 1.0e-6f);
		const float fresnel = pow(1.0f - viewDotHalfway, 5.0f);
		scale += (1.0f - fresnel) * visibility;
		bias += fresnel * visibility;
	}

	outputTexture[dispatchId.xy] = float2(scale, bias) / float(gSampleCount);
}

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

	float3 irradiance = 0.0f;
	for (uint sampleIndex = 0u; sampleIndex < gSampleCount; ++sampleIndex)
	{
		const float2 xi = Hammersley(sampleIndex, gSampleCount);
		const float phi = kTwoPi * xi.x;
		const float cosTheta = sqrt(1.0f - xi.y);
		const float sinTheta = sqrt(xi.y);
		const float3 sampleDirection = TangentToWorld(
			float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta),
			normal);
		irradiance += SampleEquirectangular(sourceTexture, gEnvironmentSampler, sampleDirection, 0.0f);
	}

	// cosine-weighted samplingのPDFを戻し、照度の半球積分値にする。
	irradiance *= kPi / float(gSampleCount);
	outputTexture[dispatchId] = float4(irradiance, 1.0f);
}

Texture2D<float4> gInputColor : register(t0);
SamplerState gLinearClampSampler : register(s0);

cbuffer PostProcessParameters : register(b0)
{
	float4 gPostProcessParameters[8];
};

cbuffer PostProcessContext : register(b1)
{
	float2 gOutputSize;
	float2 gInverseOutputSize;
	float2 gInputSize;
	float2 gInverseInputSize;
	float2 gSceneSize;
	float2 gInverseSceneSize;
	uint gFrameIndex;
	float3 gContextPadding;
};

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float4 main(FullscreenVertex input) : SV_Target0
{
	const float2 center = gPostProcessParameters[0].xy;
	const float strength = gPostProcessParameters[0].z;
	const uint sampleCount = (uint)clamp(round(gPostProcessParameters[0].w), 2.0f, 32.0f);
	const float2 uv = input.position.xy * gInverseOutputSize;
	const float2 direction = center - uv;
	float4 color = 0.0f;
	[loop]
	for (uint index = 0u; index < sampleCount; ++index)
	{
		const float offset = (float)index / (float)(sampleCount - 1u);
		color += gInputColor.SampleLevel(gLinearClampSampler, uv + direction * strength * offset, 0.0f);
	}
	return color / (float)sampleCount;
}

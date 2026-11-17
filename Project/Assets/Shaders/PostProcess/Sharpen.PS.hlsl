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

float3 SampleColor(float2 uv, float2 offset)
{
	return gInputColor.SampleLevel(gLinearClampSampler, uv + offset * gInverseInputSize, 0.0f).rgb;
}

float4 main(FullscreenVertex input) : SV_Target0
{
	const float intensity = gPostProcessParameters[0].x;
	const float threshold = gPostProcessParameters[0].y;
	const float2 uv = input.position.xy * gInverseOutputSize;
	const float4 source = gInputColor.SampleLevel(gLinearClampSampler, uv, 0.0f);
	const float3 average = (
		SampleColor(uv, float2(0.0f, -1.0f)) +
		SampleColor(uv, float2(0.0f, 1.0f)) +
		SampleColor(uv, float2(-1.0f, 0.0f)) +
		SampleColor(uv, float2(1.0f, 0.0f))) * 0.25f;
	const float3 detail = source.rgb - average;
	const float maximumDetail = max(abs(detail.r), max(abs(detail.g), abs(detail.b)));
	const float3 sharpened = saturate(source.rgb + detail * intensity);
	return float4(maximumDetail >= threshold ? sharpened : source.rgb, source.a);
}

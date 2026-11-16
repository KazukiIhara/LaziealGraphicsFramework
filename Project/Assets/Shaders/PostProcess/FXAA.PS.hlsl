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

float GetLuminance(float3 color)
{
	return dot(color, float3(0.2126f, 0.7152f, 0.0722f));
}

float3 SampleColor(float2 uv, float2 offset)
{
	return gInputColor.SampleLevel(gLinearClampSampler, uv + offset * gInverseInputSize, 0.0f).rgb;
}

float4 main(FullscreenVertex input) : SV_Target0
{
	const float edgeThreshold = gPostProcessParameters[0].x;
	const float minimumEdgeThreshold = gPostProcessParameters[0].y;
	const float blendStrength = gPostProcessParameters[0].z;
	const float2 uv = input.position.xy * gInverseOutputSize;
	const float4 source = gInputColor.SampleLevel(gLinearClampSampler, uv, 0.0f);

	const float luminanceCenter = GetLuminance(source.rgb);
	const float luminanceNorth = GetLuminance(SampleColor(uv, float2(0.0f, -1.0f)));
	const float luminanceSouth = GetLuminance(SampleColor(uv, float2(0.0f, 1.0f)));
	const float luminanceWest = GetLuminance(SampleColor(uv, float2(-1.0f, 0.0f)));
	const float luminanceEast = GetLuminance(SampleColor(uv, float2(1.0f, 0.0f)));
	const float minimumLuminance = min(luminanceCenter, min(min(luminanceNorth, luminanceSouth), min(luminanceWest, luminanceEast)));
	const float maximumLuminance = max(luminanceCenter, max(max(luminanceNorth, luminanceSouth), max(luminanceWest, luminanceEast)));
	const float luminanceRange = maximumLuminance - minimumLuminance;
	if (luminanceRange < max(minimumEdgeThreshold, maximumLuminance * edgeThreshold))
	{
		return source;
	}

	const float luminanceNorthWest = GetLuminance(SampleColor(uv, float2(-1.0f, -1.0f)));
	const float luminanceNorthEast = GetLuminance(SampleColor(uv, float2(1.0f, -1.0f)));
	const float luminanceSouthWest = GetLuminance(SampleColor(uv, float2(-1.0f, 1.0f)));
	const float luminanceSouthEast = GetLuminance(SampleColor(uv, float2(1.0f, 1.0f)));
	float2 direction = float2(
		-(luminanceNorthWest + luminanceNorthEast - luminanceSouthWest - luminanceSouthEast),
		luminanceNorthWest + luminanceSouthWest - luminanceNorthEast - luminanceSouthEast);
	const float directionReduce = max(
		(luminanceNorthWest + luminanceNorthEast + luminanceSouthWest + luminanceSouthEast) * (0.25f / 32.0f),
		1.0f / 128.0f);
	const float inverseDirectionMinimum = rcp(min(abs(direction.x), abs(direction.y)) + directionReduce);
	direction = clamp(direction * inverseDirectionMinimum, -8.0f, 8.0f) * gInverseInputSize;

	const float3 colorA = 0.5f * (
		gInputColor.SampleLevel(gLinearClampSampler, uv + direction * (1.0f / 3.0f - 0.5f), 0.0f).rgb +
		gInputColor.SampleLevel(gLinearClampSampler, uv + direction * (2.0f / 3.0f - 0.5f), 0.0f).rgb);
	const float3 colorB = colorA * 0.5f + 0.25f * (
		gInputColor.SampleLevel(gLinearClampSampler, uv + direction * -0.5f, 0.0f).rgb +
		gInputColor.SampleLevel(gLinearClampSampler, uv + direction * 0.5f, 0.0f).rgb);
	const float luminanceB = GetLuminance(colorB);
	const float3 antialiased = luminanceB < minimumLuminance || luminanceB > maximumLuminance ? colorA : colorB;
	return float4(lerp(source.rgb, antialiased, blendStrength), source.a);
}

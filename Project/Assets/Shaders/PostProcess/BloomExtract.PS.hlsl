Texture2D<float4> gInputColor : register(t0);

cbuffer PostProcessParameters : register(b0)
{
	float4 gPostProcessParameters[8];
};

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float4 main(FullscreenVertex input) : SV_Target0
{
	uint width;
	uint height;
	gInputColor.GetDimensions(width, height);
	const int2 maximumPixel = int2(width - 1u, height - 1u);
	const int2 sourcePixel = int2(input.position.xy) * 2;
	float3 color = 0.0f;
	color += gInputColor.Load(int3(clamp(sourcePixel, int2(0, 0), maximumPixel), 0)).rgb;
	color += gInputColor.Load(int3(clamp(sourcePixel + int2(1, 0), int2(0, 0), maximumPixel), 0)).rgb;
	color += gInputColor.Load(int3(clamp(sourcePixel + int2(0, 1), int2(0, 0), maximumPixel), 0)).rgb;
	color += gInputColor.Load(int3(clamp(sourcePixel + int2(1, 1), int2(0, 0), maximumPixel), 0)).rgb;
	color *= 0.25f;

	const float threshold = gPostProcessParameters[0].x;
	const float softKnee = gPostProcessParameters[0].y;
	const float brightness = max(color.r, max(color.g, color.b));
	const float knee = max(threshold * softKnee, 1.0e-5f);
	float softContribution = brightness - threshold + knee;
	softContribution = clamp(softContribution, 0.0f, 2.0f * knee);
	softContribution = softContribution * softContribution / (4.0f * knee);
	const float contribution = max(brightness - threshold, softContribution) /
		max(brightness, 1.0e-5f);
	return float4(color * contribution, 1.0f);
}

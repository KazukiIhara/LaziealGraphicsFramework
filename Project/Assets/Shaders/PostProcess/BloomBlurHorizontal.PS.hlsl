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
	const int2 pixel = int2(input.position.xy);
	const int radius = 1 + (int)round(gPostProcessParameters[0].w * 2.0f);
	float3 color = gInputColor.Load(int3(pixel, 0)).rgb * 0.227027f;
	color += gInputColor.Load(int3(clamp(pixel + int2(radius, 0), int2(0, 0), maximumPixel), 0)).rgb * 0.316216f;
	color += gInputColor.Load(int3(clamp(pixel - int2(radius, 0), int2(0, 0), maximumPixel), 0)).rgb * 0.316216f;
	color += gInputColor.Load(int3(clamp(pixel + int2(radius * 2, 0), int2(0, 0), maximumPixel), 0)).rgb * 0.070270f;
	color += gInputColor.Load(int3(clamp(pixel - int2(radius * 2, 0), int2(0, 0), maximumPixel), 0)).rgb * 0.070270f;
	return float4(color, 1.0f);
}

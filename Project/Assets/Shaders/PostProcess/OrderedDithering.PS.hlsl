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
	static const float bayerMatrix[16] =
	{
		0.0f, 8.0f, 2.0f, 10.0f,
		12.0f, 4.0f, 14.0f, 6.0f,
		3.0f, 11.0f, 1.0f, 9.0f,
		15.0f, 7.0f, 13.0f, 5.0f
	};

	const float colorSteps = max(round(gPostProcessParameters[0].x), 2.0f);
	const float intensity = gPostProcessParameters[0].y;
	const float scale = max(round(gPostProcessParameters[0].z), 1.0f);
	const float4 source = gInputColor.Load(int3(int2(input.position.xy), 0));
	const uint2 patternCoordinate = uint2(floor(input.position.xy / scale)) & 3u;
	const float threshold = (bayerMatrix[patternCoordinate.y * 4u + patternCoordinate.x] + 0.5f) / 16.0f - 0.5f;
	const float denominator = colorSteps - 1.0f;
	const float3 dithered = saturate(floor(saturate(source.rgb) * denominator + threshold + 0.5f) / denominator);
	return float4(lerp(source.rgb, dithered, intensity), source.a);
}

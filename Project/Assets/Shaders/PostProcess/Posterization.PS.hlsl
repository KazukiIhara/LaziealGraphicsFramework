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
	const float colorSteps = max(round(gPostProcessParameters[0].x), 2.0f);
	const float intensity = gPostProcessParameters[0].y;
	const float4 source = gInputColor.Load(int3(int2(input.position.xy), 0));
	const float denominator = colorSteps - 1.0f;
	const float3 posterized = round(saturate(source.rgb) * denominator) / denominator;
	return float4(lerp(source.rgb, posterized, intensity), source.a);
}

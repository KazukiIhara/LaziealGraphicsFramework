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
	const float intensity = gPostProcessParameters[0].x;
	const float4 source = gInputColor.Load(int3(int2(input.position.xy), 0));
	return float4(lerp(source.rgb, 1.0f - source.rgb, intensity), source.a);
}

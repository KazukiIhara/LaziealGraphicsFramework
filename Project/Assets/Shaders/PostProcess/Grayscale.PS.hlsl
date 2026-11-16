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
	const float luminance = dot(source.rgb, float3(0.2126f, 0.7152f, 0.0722f));
	return float4(lerp(source.rgb, luminance.xxx, intensity), source.a);
}

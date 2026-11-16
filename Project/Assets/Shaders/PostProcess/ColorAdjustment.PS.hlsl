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
	const float brightness = gPostProcessParameters[0].x;
	const float contrast = gPostProcessParameters[0].y;
	const float saturation = gPostProcessParameters[0].z;
	const float intensity = gPostProcessParameters[0].w;
	const float4 tint = gPostProcessParameters[1];

	const float4 source = gInputColor.Load(int3(int2(input.position.xy), 0));
	float3 adjusted = source.rgb + brightness;
	adjusted = (adjusted - 0.5f) * contrast + 0.5f;
	const float luminance = dot(adjusted, float3(0.2126f, 0.7152f, 0.0722f));
	adjusted = lerp(luminance.xxx, adjusted, saturation) * tint.rgb;
	return float4(lerp(source.rgb, adjusted, intensity), source.a);
}

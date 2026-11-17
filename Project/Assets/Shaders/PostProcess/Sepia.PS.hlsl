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
	const float3 sepia = saturate(float3(
		dot(source.rgb, float3(0.393f, 0.769f, 0.189f)),
		dot(source.rgb, float3(0.349f, 0.686f, 0.168f)),
		dot(source.rgb, float3(0.272f, 0.534f, 0.131f))));
	return float4(lerp(source.rgb, sepia, intensity), source.a);
}

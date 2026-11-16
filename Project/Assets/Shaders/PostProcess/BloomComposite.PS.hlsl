Texture2D<float4> gInputColor : register(t0);
Texture2D<float4> gBloomTexture : register(t1);
SamplerState gLinearClampSampler : register(s0);

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
	const float2 uv = input.position.xy / float2(width, height);
	const float4 source = gInputColor.Load(int3(int2(input.position.xy), 0));
	const float3 bloom = gBloomTexture.SampleLevel(gLinearClampSampler, uv, 0.0f).rgb;
	return float4(source.rgb + bloom * gPostProcessParameters[0].z, source.a);
}

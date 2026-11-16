Texture2D<float4> gInputColor : register(t0);
SamplerState gPointClampSampler : register(s0);

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

float4 main(FullscreenVertex input) : SV_Target0
{
	const float pixelSize = max(round(gPostProcessParameters[0].x), 1.0f);
	const float2 blockCenter = floor(input.position.xy / pixelSize) * pixelSize + pixelSize * 0.5f;
	const float2 minimumUV = gInverseInputSize * 0.5f;
	const float2 maximumUV = 1.0f - minimumUV;
	const float2 uv = clamp(blockCenter * gInverseInputSize, minimumUV, maximumUV);
	return gInputColor.SampleLevel(gPointClampSampler, uv, 0.0f);
}

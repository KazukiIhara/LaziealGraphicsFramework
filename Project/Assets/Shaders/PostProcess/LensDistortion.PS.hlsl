Texture2D<float4> gInputColor : register(t0);
SamplerState gLinearClampSampler : register(s0);

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
	const float distortion = gPostProcessParameters[0].x;
	const float scale = gPostProcessParameters[0].y;
	const float2 center = gPostProcessParameters[0].zw;
	const float2 uv = input.position.xy * gInverseOutputSize;
	float2 centered = (uv - center) / max(scale, 1.0e-4f);
	const float aspectRatio = gInputSize.x / max(gInputSize.y, 1.0f);
	centered.x *= aspectRatio;
	const float radiusSquared = dot(centered, centered);
	centered *= 1.0f + distortion * radiusSquared;
	centered.x /= aspectRatio;
	return gInputColor.SampleLevel(gLinearClampSampler, center + centered, 0.0f);
}

Texture2D<float4> gInputColor : register(t0);

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
	const float intensity = gPostProcessParameters[0].x;
	const float smoothness = gPostProcessParameters[0].y;
	const float roundness = gPostProcessParameters[0].z;
	const float4 vignetteColor = gPostProcessParameters[1];

	const float4 source = gInputColor.Load(int3(int2(input.position.xy), 0));
	const float2 uv = input.position.xy * gInverseOutputSize;
	float2 centered = abs(uv * 2.0f - 1.0f);
	const float aspectRatio = gOutputSize.x / max(gOutputSize.y, 1.0f);
	centered.x *= lerp(1.0f, aspectRatio, roundness);
	const float distanceFromCenter = length(centered);
	const float edge = smoothstep(
		max(1.0f - intensity, 0.0f),
		max(1.0f - intensity, 0.0f) + max(smoothness, 1.0e-4f),
		distanceFromCenter);
	return float4(
		lerp(source.rgb, vignetteColor.rgb, edge * vignetteColor.a),
		source.a);
}

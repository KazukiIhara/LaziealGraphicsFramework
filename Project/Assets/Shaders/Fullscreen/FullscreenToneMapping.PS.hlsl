Texture2D<float4> gSceneColor : register(t0);

cbuffer ToneMappingData : register(b0)
{
	float gExposureEV;
	float3 gToneMappingPadding;
};

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float3 ACESFilm(float3 color)
{
	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;
	return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

float4 main(FullscreenVertex input) : SV_Target0
{
	// SceneColorはLinear HDR。ExposureとACES適用後もLinearで返し、
	// R8G8B8A8_UNORM_SRGBのRTVにLinear -> sRGB変換を任せる。
	const float4 sceneColor = gSceneColor.Load(int3(int2(input.position.xy), 0));
	const float3 exposedColor = max(sceneColor.rgb, 0.0f) * exp2(gExposureEV);
	return float4(ACESFilm(exposedColor), sceneColor.a);
}

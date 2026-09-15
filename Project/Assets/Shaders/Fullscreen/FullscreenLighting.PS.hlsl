Texture2D<float4> gGBufferAlbedo : register(t0);
Texture2D<float4> gGBufferNormal : register(t1);
Texture2D<float> gDepthTexture : register(t2);

cbuffer DirectionalLightData : register(b0)
{
	float3 gDirectionalLightDirection;
	float gDirectionalLightIntensity;
	float3 gDirectionalLightColor;
	float gDirectionalLightPadding;
};

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float4 main(FullscreenVertex input) : SV_Target0
{
	int2 pixel = int2(input.position.xy);

	// GBufferの取得
	float4 albedo = gGBufferAlbedo.Load(int3(pixel, 0));
	float4 normalData = gGBufferNormal.Load(int3(pixel, 0));
	float3 normal = normalize(normalData.xyz);
	float depth = gDepthTexture.Load(int3(pixel, 0));

	// 背景ピクセルの出力
	if (depth >= 1.0f)
	{
		return albedo;
	}

	// UnlitマテリアルはGBufferの色をそのまま出力する
	if (normalData.w > 0.5f)
	{
		return albedo;
	}

	// 平行光源の計算
	float diffuse = saturate(dot(normal, -gDirectionalLightDirection));

	// Lighting結果の出力
	float3 ambient = albedo.rgb * 0.12f;
	float3 directional = albedo.rgb * gDirectionalLightColor * gDirectionalLightIntensity * diffuse;
	float3 lighting = ambient + directional;
	return float4(lighting, albedo.a);
}

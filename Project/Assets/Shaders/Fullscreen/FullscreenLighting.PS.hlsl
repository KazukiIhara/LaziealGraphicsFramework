Texture2D<float4> gGBufferAlbedo : register(t0);
Texture2D<float4> gGBufferNormal : register(t1);
Texture2D<float> gDepthTexture : register(t2);

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float4 main(FullscreenVertex input) : SV_Target0
{
	int2 pixel = int2(input.position.xy);

	// GBufferの取得
	float4 albedo = gGBufferAlbedo.Load(int3(pixel, 0));
	float3 normal = normalize(gGBufferNormal.Load(int3(pixel, 0)).xyz);
	float depth = gDepthTexture.Load(int3(pixel, 0));

	// 背景ピクセルの出力
	if (depth >= 1.0f)
	{
		return albedo;
	}

	// 平行光源の計算
	float3 lightDir = normalize(float3(0.0f, -1.0f, 0.0f));
	float diffuse = saturate(dot(normal, -lightDir));

	// Lighting結果の出力
	float3 ambient = albedo.rgb * 0.12f;
	float3 lighting = ambient + albedo.rgb * diffuse;
	return float4(lighting, albedo.a);
}

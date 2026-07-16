Texture2D<float> gDepthTexture : register(t0);

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float4 main(FullscreenVertex input) : SV_Target0
{
	// Depth値の取得
	float depth = gDepthTexture.Load(int3(int2(input.position.xy), 0));

	// Depth値の出力
	return float4(depth, depth, depth, 1.0f);
}

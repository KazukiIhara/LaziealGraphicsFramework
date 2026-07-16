Texture2D<float4> gSceneColor : register(t0);

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float4 main(FullscreenVertex input) : SV_Target0
{
	// PixelShader内のSV_Positionはすでにピクセル座標になっている。
	// Loadを使うことでSamplerをRootSignatureに追加せず、まずは単純な画面転送にする。
	return gSceneColor.Load(int3(int2(input.position.xy), 0));
}

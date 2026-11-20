struct FullscreenVertex
{
	float4 position : SV_Position;
};

cbuffer SceneTransitionParameters : register(b0)
{
	float4 gColor;
	float4 gParameters[4];
	// x: 遷移全体の進行度、y: イージング適用後のフェーズ進行度、z: 不透明度、w: フェーズ
	float4 gContext;
	// xy: 画面サイズ、zw: 画面サイズの逆数
	float4 gScreen;
};

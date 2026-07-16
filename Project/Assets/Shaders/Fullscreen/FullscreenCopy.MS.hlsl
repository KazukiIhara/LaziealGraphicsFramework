struct FullscreenVertex
{
	float4 position : SV_Position;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
	out vertices FullscreenVertex vertices[3],
	out indices uint3 triangles[1])
{
	SetMeshOutputCounts(3, 1);

	// 画面より大きい三角形を1枚だけ描画して、画面全体を覆う。
	// 2枚の三角形で描く場合と違い、中央に斜めの境界が出ないのでポストエフェクト向き。
	vertices[0].position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
	vertices[1].position = float4(-1.0f,  3.0f, 0.0f, 1.0f);
	vertices[2].position = float4( 3.0f, -1.0f, 0.0f, 1.0f);

	triangles[0] = uint3(0, 1, 2);
}

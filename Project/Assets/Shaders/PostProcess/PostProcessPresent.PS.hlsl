Texture2D<float4> gInputColor : register(t0);

struct FullscreenVertex
{
	float4 position : SV_Position;
};

float4 main(FullscreenVertex input) : SV_Target0
{
	// Linear LDRを返し、SwapChainのsRGB RTVにLinearからsRGBへの変換を任せる。
	return gInputColor.Load(int3(int2(input.position.xy), 0));
}

#include "SceneTransition.hlsli"

float4 main(FullscreenVertex input) : SV_Target0
{
	return float4(gColor.rgb, gColor.a * gContext.z);
}

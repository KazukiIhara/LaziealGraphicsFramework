#include "SceneTransition.hlsli"

float4 main(FullscreenVertex input) : SV_Target0
{
	if (gContext.y <= 0.0f)
	{
		return float4(gColor.rgb, gColor.a * gContext.w);
	}
	if (gContext.y >= 1.0f)
	{
		return float4(gColor.rgb, gColor.a * (1.0f - gContext.w));
	}

	const float uvX = input.position.x * gScreen.z;
	const float softness = max(gParameters[0].x, 0.0001f);
	const float edgeStart = gContext.y - softness;
	const float edgeEnd = gContext.y + softness;
	const float opacity = gContext.w < 0.5f
		? 1.0f - smoothstep(edgeStart, edgeEnd, uvX)
		: smoothstep(edgeStart, edgeEnd, uvX);
	return float4(gColor.rgb, gColor.a * opacity);
}

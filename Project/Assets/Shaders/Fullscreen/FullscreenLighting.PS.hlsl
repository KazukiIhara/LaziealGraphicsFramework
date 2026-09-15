Texture2D<float4> gGBufferAlbedo : register(t0);
Texture2D<float4> gGBufferNormal : register(t1);
Texture2D<float4> gGBufferMaterial : register(t2);
Texture2D<float> gDepthTexture : register(t3);

struct CameraData3D
{
	float4x4 viewProj;
	float4x4 inverseViewProj;
	float3 worldPos;
	float padding;
	float4 frustumPlanes[6];
};

ConstantBuffer<CameraData3D> gCamera : register(b1);

cbuffer SkyboxData : register(b2)
{
	uint gSkyboxTextureIndex;
	float gSkyboxIntensity;
	float gSkyboxRotation;
	float gSkyboxPadding;
};

SamplerState gSkyboxSampler : register(s0);

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

static const float kPi = 3.14159265359f;
static const float kTwoPi = kPi * 2.0f;

float3 FresnelSchlick(float cosTheta, float3 reflectanceAtNormal)
{
	return reflectanceAtNormal +
		(1.0f - reflectanceAtNormal) * pow(1.0f - cosTheta, 5.0f);
}

float DistributionGGX(float3 normal, float3 halfway, float roughness)
{
	const float alpha = roughness * roughness;
	const float alphaSquared = alpha * alpha;
	const float normalDotHalfway = saturate(dot(normal, halfway));
	const float denominator =
		normalDotHalfway * normalDotHalfway * (alphaSquared - 1.0f) + 1.0f;
	return alphaSquared / max(kPi * denominator * denominator, 1.0e-6f);
}

float GeometrySchlickGGX(float normalDotDirection, float roughness)
{
	const float remappedRoughness = roughness + 1.0f;
	const float k = remappedRoughness * remappedRoughness / 8.0f;
	return normalDotDirection /
		max(normalDotDirection * (1.0f - k) + k, 1.0e-6f);
}

float GeometrySmith(float3 normal, float3 view, float3 light, float roughness)
{
	return GeometrySchlickGGX(saturate(dot(normal, view)), roughness) *
		GeometrySchlickGGX(saturate(dot(normal, light)), roughness);
}

float3 ReconstructWorldPosition(int2 pixel, float depth)
{
	uint width;
	uint height;
	gDepthTexture.GetDimensions(width, height);
	const float2 uv = (float2(pixel) + 0.5f) / float2(width, height);
	const float4 clipPosition = float4(
		uv.x * 2.0f - 1.0f,
		1.0f - uv.y * 2.0f,
		depth,
		1.0f);
	const float4 worldPosition = mul(clipPosition, gCamera.inverseViewProj);
	return worldPosition.xyz / max(worldPosition.w, 1.0e-6f);
}

float3 SampleSkybox(int2 pixel)
{
	if (gSkyboxTextureIndex == 0xffffffffu)
	{
		return 0.0f;
	}

	const float3 farPosition = ReconstructWorldPosition(pixel, 1.0f);
	const float3 direction = normalize(farPosition - gCamera.worldPos);
	float2 uv;
	uv.x = frac(atan2(direction.x, direction.z) / kTwoPi + 0.5f + gSkyboxRotation / kTwoPi);
	uv.y = acos(clamp(direction.y, -1.0f, 1.0f)) / kPi;

	Texture2D<float4> skyboxTexture = ResourceDescriptorHeap[gSkyboxTextureIndex];
	uint textureWidth;
	uint textureHeight;
	skyboxTexture.GetDimensions(textureWidth, textureHeight);
	const float halfTexelY = 0.5f / max(float(textureHeight), 1.0f);
	uv.y = clamp(uv.y, halfTexelY, 1.0f - halfTexelY);
	return max(skyboxTexture.SampleLevel(gSkyboxSampler, uv, 0.0f).rgb, 0.0f) * gSkyboxIntensity;
}

float4 main(FullscreenVertex input) : SV_Target0
{
	int2 pixel = int2(input.position.xy);

	// GBufferの取得
	float4 albedo = gGBufferAlbedo.Load(int3(pixel, 0));
	float4 normalData = gGBufferNormal.Load(int3(pixel, 0));
	float3 normal = normalize(normalData.xyz);
	float3 material = gGBufferMaterial.Load(int3(pixel, 0)).rgb;
	float depth = gDepthTexture.Load(int3(pixel, 0));

	// 背景ピクセルの出力
	if (depth >= 1.0f)
	{
		return gSkyboxTextureIndex == 0xffffffffu
			? albedo
			: float4(SampleSkybox(pixel), 1.0f);
	}

	// UnlitマテリアルはGBufferの色をそのまま出力する
	if (normalData.w > 0.5f)
	{
		return albedo;
	}

	const float ambientOcclusion = saturate(material.r);
	const float roughness = clamp(material.g, 0.045f, 1.0f);
	const float metallic = saturate(material.b);
	const float3 worldPosition = ReconstructWorldPosition(pixel, depth);
	const float3 viewDirection = normalize(gCamera.worldPos - worldPosition);
	const float3 lightDirection = normalize(-gDirectionalLightDirection);
	const float3 halfwayDirection = normalize(viewDirection + lightDirection);
	const float normalDotLight = saturate(dot(normal, lightDirection));
	const float normalDotView = saturate(dot(normal, viewDirection));

	const float3 reflectanceAtNormal = lerp(float3(0.04f, 0.04f, 0.04f), albedo.rgb, metallic);
	const float3 fresnel = FresnelSchlick(
		saturate(dot(halfwayDirection, viewDirection)),
		reflectanceAtNormal);
	const float distribution = DistributionGGX(normal, halfwayDirection, roughness);
	const float geometry = GeometrySmith(normal, viewDirection, lightDirection, roughness);
	const float3 specular =
		distribution * geometry * fresnel /
		max(4.0f * normalDotView * normalDotLight, 1.0e-6f);

	const float3 diffuseWeight = (1.0f - fresnel) * (1.0f - metallic);
	const float3 diffuse = diffuseWeight * albedo.rgb / kPi;
	const float3 radiance = gDirectionalLightColor * gDirectionalLightIntensity;
	const float3 directLighting = (diffuse + specular) * radiance * normalDotLight;
	// IBL実装前の最低限の環境光。AOは間接光だけに適用する。
	const float3 ambient = albedo.rgb * 0.03f * ambientOcclusion;
	const float3 lighting = ambient + directLighting;
	return float4(lighting, albedo.a);
}

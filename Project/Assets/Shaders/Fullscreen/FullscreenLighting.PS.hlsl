Texture2D<float4> gGBufferAlbedo : register(t0);
Texture2D<float4> gGBufferNormal : register(t1);
Texture2D<float4> gGBufferMaterial : register(t2);
Texture2D<float> gDepthTexture : register(t3);
Texture2D<float> gShadowMap[4] : register(t4);

struct CameraData3D
{
	float4x4 viewProj;
	float4x4 inverseViewProj;
	float3 worldPos;
	float padding;
	float4 frustumPlanes[6];
};

ConstantBuffer<CameraData3D> gCamera : register(b1);

cbuffer gShadowCamera : register(b3)
{
	float4x4 gShadowViewProjections[4];
	float4x4 gShadowView;
	float4 gCascadeSplitDepths;
	float4 gShadowSettings;
};

cbuffer SkyboxData : register(b2)
{
	uint gSkyboxTextureIndex;
	uint gIrradianceTextureIndex;
	uint gPrefilteredTextureIndex;
	uint gBrdfLutTextureIndex;
	float gSkyboxIntensity;
	float gSkyboxRotation;
	uint gPrefilteredMipCount;
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

float SampleCascadeShadow(
	uint cascadeIndex,
	float3 worldPosition,
	float3 normal,
	float3 lightDirection)
{
	const float4 lightClip = mul(
		float4(worldPosition, 1.0f),
		gShadowViewProjections[cascadeIndex]);
	if (lightClip.w <= 0.0f)
	{
		return 1.0f;
	}

	const float3 lightNdc = lightClip.xyz / lightClip.w;
	const float2 shadowUv = float2(lightNdc.x * 0.5f + 0.5f, 0.5f - lightNdc.y * 0.5f);
	if (any(shadowUv < 0.0f) || any(shadowUv > 1.0f) || lightNdc.z < 0.0f || lightNdc.z > 1.0f)
	{
		return 1.0f;
	}

	uint shadowWidth;
	uint shadowHeight;
	const uint shadowMapIndex = NonUniformResourceIndex(cascadeIndex);
	gShadowMap[shadowMapIndex].GetDimensions(shadowWidth, shadowHeight);
	const int2 maximumPixel = int2(shadowWidth - 1u, shadowHeight - 1u);
	const int2 shadowPixel = clamp(
		int2(shadowUv * float2(shadowWidth, shadowHeight)),
		int2(0, 0),
		maximumPixel);
	const float bias = max(
		gShadowSettings.x * (1.0f - saturate(dot(normal, lightDirection))),
		gShadowSettings.x * 0.25f);

	float visibility = 0.0f;
	[unroll]
	for (int y = -1; y <= 1; ++y)
	{
		[unroll]
		for (int x = -1; x <= 1; ++x)
		{
			const int2 samplePixel = clamp(
				shadowPixel + int2(x, y),
				int2(0, 0),
				maximumPixel);
			const float storedDepth = gShadowMap[shadowMapIndex].Load(
				int3(samplePixel, 0));
			visibility += lightNdc.z - bias <= storedDepth ? 1.0f : 0.0f;
		}
	}
	return visibility / 9.0f;
}

float CalculateShadow(float3 worldPosition, float3 normal, float3 lightDirection)
{
	const float viewDepth = mul(float4(worldPosition, 1.0f), gShadowView).z;
	if (viewDepth < gShadowSettings.z || viewDepth > gCascadeSplitDepths.w)
	{
		return 1.0f;
	}

	uint cascadeIndex = 0u;
	cascadeIndex += viewDepth > gCascadeSplitDepths.x;
	cascadeIndex += viewDepth > gCascadeSplitDepths.y;
	cascadeIndex += viewDepth > gCascadeSplitDepths.z;

	const float shadow = SampleCascadeShadow(
		cascadeIndex,
		worldPosition,
		normal,
		lightDirection);
	if (cascadeIndex + 1u >= 4u)
	{
		return shadow;
	}

	const float cascadeNear = cascadeIndex == 0u
		? gShadowSettings.z
		: gCascadeSplitDepths[cascadeIndex - 1u];
	const float cascadeFar = gCascadeSplitDepths[cascadeIndex];
	const float blendWidth = (cascadeFar - cascadeNear) * gShadowSettings.y;
	const float blendAmount = saturate(
		(viewDepth - (cascadeFar - blendWidth)) / max(blendWidth, 1.0e-6f));
	if (blendAmount <= 0.0f)
	{
		return shadow;
	}

	const float nextShadow = SampleCascadeShadow(
		cascadeIndex + 1u,
		worldPosition,
		normal,
		lightDirection);
	return lerp(shadow, nextShadow, blendAmount);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 reflectanceAtNormal, float roughness)
{
	return reflectanceAtNormal +
		(max(1.0f - roughness, reflectanceAtNormal) - reflectanceAtNormal) *
		pow(1.0f - cosTheta, 5.0f);
}

float3 RotateEnvironmentDirection(float3 direction)
{
	const float cosine = cos(gSkyboxRotation);
	const float sine = sin(gSkyboxRotation);
	return float3(
		direction.x * cosine + direction.z * sine,
		direction.y,
		-direction.x * sine + direction.z * cosine);
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
	const float shadow = CalculateShadow(worldPosition, normal, lightDirection);
	const float3 directLighting = (diffuse + specular) * radiance * normalDotLight * shadow;
	float3 ambient = albedo.rgb * 0.03f * ambientOcclusion;
	const bool hasIbl =
		gIrradianceTextureIndex != 0xffffffffu &&
		gPrefilteredTextureIndex != 0xffffffffu &&
		gBrdfLutTextureIndex != 0xffffffffu &&
		gPrefilteredMipCount > 0u;
	if (hasIbl)
	{
		TextureCube<float4> irradianceTexture = ResourceDescriptorHeap[gIrradianceTextureIndex];
		TextureCube<float4> prefilteredTexture = ResourceDescriptorHeap[gPrefilteredTextureIndex];
		Texture2D<float2> brdfLutTexture = ResourceDescriptorHeap[gBrdfLutTextureIndex];

		const float3 environmentNormal = RotateEnvironmentDirection(normal);
		const float3 reflectionDirection = RotateEnvironmentDirection(reflect(-viewDirection, normal));
		const float3 environmentFresnel = FresnelSchlickRoughness(
			normalDotView,
			reflectanceAtNormal,
			roughness);
		const float3 environmentDiffuseWeight =
			(1.0f - environmentFresnel) * (1.0f - metallic);
		const float3 irradiance = irradianceTexture.SampleLevel(
			gSkyboxSampler,
			environmentNormal,
			0.0f).rgb;
		const float3 diffuseIbl = environmentDiffuseWeight * albedo.rgb * irradiance / kPi;

		const float prefilteredMip = roughness * float(gPrefilteredMipCount - 1u);
		const float3 prefilteredColor = prefilteredTexture.SampleLevel(
			gSkyboxSampler,
			reflectionDirection,
			prefilteredMip).rgb;
		uint brdfWidth;
		uint brdfHeight;
		brdfLutTexture.GetDimensions(brdfWidth, brdfHeight);
		const float2 brdfHalfTexel = 0.5f / max(float2(brdfWidth, brdfHeight), 1.0f);
		const float2 brdfUv = clamp(
			float2(normalDotView, roughness),
			brdfHalfTexel,
			1.0f - brdfHalfTexel);
		const float2 brdf = brdfLutTexture.SampleLevel(gSkyboxSampler, brdfUv, 0.0f);
		const float3 specularIbl = prefilteredColor *
			(environmentFresnel * brdf.x + brdf.y);

		ambient = (diffuseIbl + specularIbl) *
			gSkyboxIntensity * ambientOcclusion;
	}
	const float3 lighting = ambient + directLighting;
	return float4(lighting, albedo.a);
}

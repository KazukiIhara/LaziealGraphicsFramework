static const float kPi = 3.14159265359f;
static const float kTwoPi = kPi * 2.0f;

float RadicalInverseVanDerCorput(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10f;
}

float2 Hammersley(uint index, uint sampleCount)
{
	return float2(float(index) / float(sampleCount), RadicalInverseVanDerCorput(index));
}

float3 CubemapDirection(uint face, float2 uv)
{
	const float2 position = uv * 2.0f - 1.0f;
	switch (face)
	{
	case 0u: return normalize(float3(1.0f, -position.y, -position.x));
	case 1u: return normalize(float3(-1.0f, -position.y, position.x));
	case 2u: return normalize(float3(position.x, 1.0f, position.y));
	case 3u: return normalize(float3(position.x, -1.0f, -position.y));
	case 4u: return normalize(float3(position.x, -position.y, 1.0f));
	default: return normalize(float3(-position.x, -position.y, -1.0f));
	}
}

float2 DirectionToEquirectUV(float3 direction)
{
	direction = normalize(direction);
	return float2(
		frac(atan2(direction.x, direction.z) / kTwoPi + 0.5f),
		acos(clamp(direction.y, -1.0f, 1.0f)) / kPi);
}

float3 SampleEquirectangular(
	Texture2D<float4> textureMap,
	SamplerState textureSampler,
	float3 direction,
	float mipLevel)
{
	float2 uv = DirectionToEquirectUV(direction);
	uint width;
	uint height;
	uint mipCount;
	textureMap.GetDimensions(0u, width, height, mipCount);
	const float halfTexelY = 0.5f / max(float(height), 1.0f);
	uv.y = clamp(uv.y, halfTexelY, 1.0f - halfTexelY);
	return max(textureMap.SampleLevel(textureSampler, uv, mipLevel).rgb, 0.0f);
}

float3 BuildTangent(float3 normal)
{
	const float3 up = abs(normal.y) < 0.999f
		? float3(0.0f, 1.0f, 0.0f)
		: float3(1.0f, 0.0f, 0.0f);
	return normalize(cross(up, normal));
}

float3 TangentToWorld(float3 localDirection, float3 normal)
{
	const float3 tangent = BuildTangent(normal);
	const float3 bitangent = cross(normal, tangent);
	return normalize(
		tangent * localDirection.x +
		bitangent * localDirection.y +
		normal * localDirection.z);
}

float3 ImportanceSampleGGX(float2 xi, float3 normal, float roughness)
{
	const float alpha = roughness * roughness;
	const float phi = kTwoPi * xi.x;
	const float cosTheta = sqrt((1.0f - xi.y) / max(1.0f + (alpha * alpha - 1.0f) * xi.y, 1.0e-6f));
	const float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
	return TangentToWorld(
		float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta),
		normal);
}

float DistributionGGX(float normalDotHalfway, float roughness)
{
	const float alpha = roughness * roughness;
	const float alphaSquared = alpha * alpha;
	const float denominator =
		normalDotHalfway * normalDotHalfway * (alphaSquared - 1.0f) + 1.0f;
	return alphaSquared / max(kPi * denominator * denominator, 1.0e-6f);
}

float GeometrySchlickGGXIBL(float normalDotDirection, float roughness)
{
	const float k = roughness * roughness * 0.5f;
	return normalDotDirection /
		max(normalDotDirection * (1.0f - k) + k, 1.0e-6f);
}

float GeometrySmithIBL(float normalDotView, float normalDotLight, float roughness)
{
	return GeometrySchlickGGXIBL(normalDotView, roughness) *
		GeometrySchlickGGXIBL(normalDotLight, roughness);
}

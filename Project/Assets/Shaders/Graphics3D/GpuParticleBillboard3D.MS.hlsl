#include "Graphics3D.hlsli"
#include "GpuParticle3D.hlsli"
#include "ParticleBillboard3D.hlsli"

ConstantBuffer<CameraData3D> gCamera : register(b0);
RWStructuredBuffer<GpuParticleData> gParticles : register(u0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID,
    out vertices ParticleBillboardOutput vertices[4],
    out indices uint3 triangles[2])
{
    GpuParticleData particle = gParticles[id.x];
	SetMeshOutputCounts(particle.alive != 0u ? 4u : 0u, particle.alive != 0u ? 2u : 0u);
    if (particle.alive == 0u)
    {
        return;
    }
    const float t = saturate(particle.age / particle.lifetime);
    const float4 color = lerp(particle.startColor, particle.endColor, t);
    const float2 size = lerp(particle.startSize, particle.endSize, t);
    const float3 right = normalize(mul(float4(1, 0, 0, 0), gCamera.inverseViewProj).xyz);
    const float3 up = normalize(mul(float4(0, 1, 0, 0), gCamera.inverseViewProj).xyz);
    float sine, cosine;
    sincos(particle.rotation, sine, cosine);
    for (uint i = 0u; i < 4u; ++i)
    {
        const float2 corner = (float2(i % 2u, i / 2u) - 0.5f) * size;
        const float2 rotated = float2(
            corner.x * cosine - corner.y * sine,
            corner.x * sine + corner.y * cosine);
        const float3 worldPosition = particle.position + right * rotated.x + up * rotated.y;
        vertices[i].position = mul(float4(worldPosition, 1), gCamera.viewProj);
        vertices[i].color = color;
        vertices[i].uv = float2(i % 2u, 1u - i / 2u);
        vertices[i].textureIndex = particle.textureIndex;
    }
    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(1, 3, 2);
}

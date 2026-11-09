#include "Graphics3D.hlsli"
#include "ParticleBillboard3D.hlsli"

struct ParticleBillboardData
{
    float3 position;
    float rotation;
    float2 size;
    uint textureIndex;
    float padding;
    float4 color;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<ParticleBillboardData> gParticle : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID,
    out vertices ParticleBillboardOutput vertices[4],
    out indices uint3 triangles[2])
{
    SetMeshOutputCounts(4, 2);
    ParticleBillboardData particle = gParticle[id.x];
    float3 right = normalize(mul(float4(1, 0, 0, 0), gCamera.inverseViewProj).xyz);
    float3 up = normalize(mul(float4(0, 1, 0, 0), gCamera.inverseViewProj).xyz);
    float sine, cosine;
    sincos(particle.rotation, sine, cosine);
    for (uint i = 0; i < 4; ++i)
    {
        float2 corner = (float2(i % 2, i / 2) - 0.5f) * particle.size;
        float2 rotated = float2(corner.x * cosine - corner.y * sine,
            corner.x * sine + corner.y * cosine);
        float3 worldPosition = particle.position + right * rotated.x + up * rotated.y;
        vertices[i].position = mul(float4(worldPosition, 1), gCamera.viewProj);
        vertices[i].color = particle.color;
        vertices[i].uv = float2(i % 2, 1 - i / 2);
        vertices[i].textureIndex = particle.textureIndex;
    }
    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(1, 3, 2);
}

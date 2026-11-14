#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);
RWStructuredBuffer<float4> gCorrections : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount || gParticles[id.x].alive == 0u)
    {
        return;
    }
    gParticles[id.x].position += gCorrections[id.x].xyz;
    gCorrections[id.x] = 0.0f;
}

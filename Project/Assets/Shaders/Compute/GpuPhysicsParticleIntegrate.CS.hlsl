#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gMaximumParticleCount)
    {
        return;
    }
    GpuPhysicsParticleData particle = gParticles[id.x];
    if (particle.alive == 0u || particle.enabled == 0u)
    {
        return;
    }
    particle.previousPosition = particle.position;
    if (particle.inverseMass <= 0.0f)
    {
        particle.velocity = 0.0f;
    }
    else
    {
        particle.velocity += (gGravity + particle.force * particle.inverseMass) * gDeltaTime;
        particle.position += particle.velocity * gDeltaTime;
    }
    gParticles[id.x] = particle;
}

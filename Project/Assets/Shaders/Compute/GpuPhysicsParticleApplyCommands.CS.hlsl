#include "GpuPhysicsParticle.hlsli"

RWStructuredBuffer<GpuPhysicsParticleData> gParticles : register(u0);
StructuredBuffer<GpuPhysicsParticleCommandRange> gCommandRanges : register(t0);
StructuredBuffer<GpuPhysicsParticleCommand> gCommands : register(t1);
RWStructuredBuffer<GpuPhysicsParticleCounters> gCounters : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= gCommandRangeCount)
    {
        return;
    }
    const GpuPhysicsParticleCommandRange range = gCommandRanges[id.x];
    const uint commandEnd = range.firstCommand + range.commandCount;
    for (uint commandIndex = range.firstCommand; commandIndex < commandEnd; ++commandIndex)
    {
        const GpuPhysicsParticleCommand command = gCommands[commandIndex];
        if (command.index >= gMaximumParticleCount)
        {
            continue;
        }
        GpuPhysicsParticleData particle = gParticles[command.index];
        if (command.type == 0u)
        {
            if (particle.alive == 0u)
            {
                InterlockedAdd(gCounters[0].aliveCount, 1u);
            }
            particle = (GpuPhysicsParticleData)0;
            particle.position = command.vectorValue;
            particle.previousPosition = command.vectorValue;
            particle.radius = command.scalarValue;
            particle.generation = command.generation;
            particle.restitution = command.restitution;
            particle.friction = command.friction;
            particle.enabled = command.boolValue;
			particle.renderGroupIndex = command.renderGroupIndex;
            particle.alive = 1u;
            gParticles[command.index] = particle;
            continue;
        }
        if (particle.alive == 0u || particle.generation != command.generation)
        {
            continue;
        }
        if (command.type == 1u)
        {
            particle.alive = 0u;
            InterlockedAdd(gCounters[0].aliveCount, 0xffffffffu);
        }
        else if (command.type == 2u)
        {
            particle.position = command.vectorValue;
            particle.previousPosition = command.vectorValue;
            if (command.boolValue != 0u)
            {
                particle.velocity = 0.0f;
            }
        }
        else if (command.type == 3u)
        {
            particle.velocity = command.vectorValue;
        }
        else if (command.type == 4u)
        {
            particle.inverseMass = command.scalarValue > 0.0f ? rcp(command.scalarValue) : 0.0f;
            if (particle.inverseMass == 0.0f)
            {
                particle.velocity = 0.0f;
            }
        }
        else if (command.type == 5u)
        {
            particle.radius = command.scalarValue;
        }
        else if (command.type == 6u)
        {
            particle.restitution = command.restitution;
            particle.friction = command.friction;
        }
        else if (command.type == 7u)
        {
            particle.enabled = command.boolValue;
        }
        else if (command.type == 8u)
        {
            particle.force += command.vectorValue;
        }
        else if (command.type == 9u && particle.inverseMass > 0.0f)
        {
            particle.velocity += command.vectorValue * particle.inverseMass;
        }
		else if (command.type == 10u)
		{
			particle.renderGroupIndex = command.renderGroupIndex;
		}
        gParticles[command.index] = particle;
    }
}

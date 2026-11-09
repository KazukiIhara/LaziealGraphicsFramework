#include "GpuParticle3D.hlsli"

RWStructuredBuffer<uint> gAliveCount : register(u0);
RWStructuredBuffer<GpuParticleDispatchArguments> gDrawArguments : register(u1);

[numthreads(1, 1, 1)]
void main()
{
    // 現段階では1 Mesh Shader Groupが1粒子を描画する。
    // AliveCountが0ならDispatchMeshも0 Groupとなり、描画Shaderは起動しない。
    gDrawArguments[0].threadGroupCountX = gAliveCount[0];
    gDrawArguments[0].threadGroupCountY = 1u;
    gDrawArguments[0].threadGroupCountZ = 1u;
}

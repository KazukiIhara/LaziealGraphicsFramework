#include "Graphics3D.hlsli"

struct ModelVertex
{
    float3 position;
    float3 normal;
    float2 uv;
};

struct ModelDrawData
{
    float4x4 worldMatrix;
    float4x4 worldInverseTranspose;
    float4 baseColor;
    uint vertexBufferIndex;
    uint meshletBufferIndex;
    uint meshletVertexIndexBufferIndex;
    uint meshletPrimitiveBufferIndex;
    uint baseColorTextureIndex;
    uint meshletCount;
    uint2 padding;
};

struct Meshlet
{
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint primitiveOffset;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
ConstantBuffer<ModelDrawData> gModel : register(b1);

static const uint kMaxMeshletVertices = 128;
static const uint kMaxMeshletPrimitives = 128;

[outputtopology("triangle")]
[numthreads(kMaxMeshletVertices, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    out vertices MSOutput3D outputVertices[kMaxMeshletVertices],
    out indices uint3 outputTriangles[kMaxMeshletPrimitives])
{
    StructuredBuffer<ModelVertex> vertexBuffer = ResourceDescriptorHeap[gModel.vertexBufferIndex];
    StructuredBuffer<Meshlet> meshletBuffer = ResourceDescriptorHeap[gModel.meshletBufferIndex];
    StructuredBuffer<uint> meshletVertexIndexBuffer =
        ResourceDescriptorHeap[gModel.meshletVertexIndexBufferIndex];
    StructuredBuffer<uint> meshletPrimitiveBuffer =
        ResourceDescriptorHeap[gModel.meshletPrimitiveBufferIndex];

    const Meshlet meshlet = meshletBuffer[groupId.x];
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    const uint localIndex = groupThreadId.x;
    if (localIndex < meshlet.vertexCount)
    {
        const uint vertexIndex = meshletVertexIndexBuffer[meshlet.vertexOffset + localIndex];
        const ModelVertex vertex = vertexBuffer[vertexIndex];

        InstanceData3D instance;
        instance.worldMatrix = gModel.worldMatrix;
        instance.worldInverseTranspose = gModel.worldInverseTranspose;
        instance.baseColor = gModel.baseColor;
        instance.uvScale = float2(1.0f, 1.0f);
        instance.uvOffset = float2(0.0f, 0.0f);
        instance.textureIndex = gModel.baseColorTextureIndex;
        instance.padding = uint3(0, 0, 0);

        outputVertices[localIndex] = MakeVertex(
            instance,
            gCamera,
            vertex.position,
            vertex.normal,
            vertex.uv);
    }

    if (localIndex < meshlet.primitiveCount)
    {
        const uint packedPrimitive =
            meshletPrimitiveBuffer[meshlet.primitiveOffset + localIndex];
        outputTriangles[localIndex] = uint3(
            packedPrimitive & 0x3ffu,
            (packedPrimitive >> 10) & 0x3ffu,
            (packedPrimitive >> 20) & 0x3ffu);
    }
}

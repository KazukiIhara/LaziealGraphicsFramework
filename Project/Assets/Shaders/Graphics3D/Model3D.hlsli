static const uint kMeshletsPerAmplificationGroup = 32;
static const uint kModelDebugShowMeshletColors = 1u << 0;

struct ModelDrawData
{
    float4x4 worldMatrix;
    float4x4 worldInverseTranspose;
    float4 baseColor;
    uint vertexBufferIndex;
    uint meshletBufferIndex;
    uint meshletVertexIndexBufferIndex;
    uint meshletPrimitiveBufferIndex;
    uint meshletCullDataBufferIndex;
    uint baseColorTextureIndex;
    uint meshletCount;
    uint meshletIndexOffset;
    uint debugFlags;
    float worldMaximumScale;
    uint2 padding;
};

struct Meshlet
{
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint primitiveOffset;
};

struct MeshletCullData
{
    float3 center;
    float radius;
    uint normalCone;
    float apexOffset;
};

struct ModelPayload
{
    uint meshletIndices[kMeshletsPerAmplificationGroup];
};

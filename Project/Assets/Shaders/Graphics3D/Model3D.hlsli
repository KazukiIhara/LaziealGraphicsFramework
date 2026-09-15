static const uint kMeshletsPerAmplificationGroup = 32;
static const uint kModelDebugShowMeshletColors = 1u << 0;
static const uint kModelDisableNormalMap = 1u << 1;
static const uint kModelUnlit = 1u << 2;

struct ModelDrawData
{
    float4 meshBaseColor;
    uint vertexBufferIndex;
    uint meshletBufferIndex;
    uint meshletVertexIndexBufferIndex;
    uint meshletPrimitiveBufferIndex;
    uint meshletCullDataBufferIndex;
    uint baseColorTextureIndex;
    uint normalTextureIndex;
    float normalScale;
    uint instanceBufferIndex;
    uint instanceOffset;
    uint instanceCount;
    uint meshletCount;
    uint meshletIndexOffset;
    uint baseWorkIndexLow;
    uint baseWorkIndexHigh;
    uint padding;
};

struct ModelInstanceData
{
    float4x4 worldMatrix;
    float4x4 worldInverseTranspose;
    float4 color;
    float worldMaximumScale;
    uint drawFlags;
    float worldOrientationSign;
    float normalScale;
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

struct VisibleModelMeshlet
{
    uint instanceIndex;
    uint meshletIndex;
};

struct ModelPayload
{
    VisibleModelMeshlet visibleMeshlets[kMeshletsPerAmplificationGroup];
};

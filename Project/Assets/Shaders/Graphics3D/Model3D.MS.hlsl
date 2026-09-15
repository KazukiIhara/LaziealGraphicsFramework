#include "Graphics3D.hlsli"
#include "Model3D.hlsli"

struct ModelVertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float4 tangent;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
ConstantBuffer<ModelDrawData> gModel : register(b1);

static const uint kMaxMeshletVertices = 128;
static const uint kMaxMeshletPrimitives = 128;

float4 GetMeshletDebugColor(uint meshletIndex)
{
    const float hue = frac(float(meshletIndex) * 0.61803398875f);
    const float3 rgb = saturate(
        abs(frac(hue + float3(0.0f, 2.0f / 3.0f, 1.0f / 3.0f)) * 6.0f - 3.0f) - 1.0f);
    return float4(lerp(float3(1.0f, 1.0f, 1.0f), rgb, 0.75f), 1.0f);
}

[outputtopology("triangle")]
[numthreads(kMaxMeshletVertices, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    in payload ModelPayload payload,
    out vertices MSOutput3D outputVertices[kMaxMeshletVertices],
    out indices uint3 outputTriangles[kMaxMeshletPrimitives])
{
    StructuredBuffer<ModelVertex> vertexBuffer = ResourceDescriptorHeap[gModel.vertexBufferIndex];
    StructuredBuffer<Meshlet> meshletBuffer = ResourceDescriptorHeap[gModel.meshletBufferIndex];
    StructuredBuffer<uint> meshletVertexIndexBuffer =
        ResourceDescriptorHeap[gModel.meshletVertexIndexBufferIndex];
    StructuredBuffer<uint> meshletPrimitiveBuffer =
        ResourceDescriptorHeap[gModel.meshletPrimitiveBufferIndex];
    StructuredBuffer<ModelInstanceData> instanceBuffer =
        ResourceDescriptorHeap[gModel.instanceBufferIndex];

    const VisibleModelMeshlet visibleMeshlet = payload.visibleMeshlets[groupId.x];
    const uint meshletIndex = visibleMeshlet.meshletIndex;
    const ModelInstanceData modelInstance = instanceBuffer[visibleMeshlet.instanceIndex];
    const Meshlet meshlet = meshletBuffer[meshletIndex];
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    const uint localIndex = groupThreadId.x;
    if (localIndex < meshlet.vertexCount)
    {
        const uint vertexIndex = meshletVertexIndexBuffer[meshlet.vertexOffset + localIndex];
        const ModelVertex vertex = vertexBuffer[vertexIndex];

        InstanceData3D instance;
        instance.worldMatrix = modelInstance.worldMatrix;
        instance.worldInverseTranspose = modelInstance.worldInverseTranspose;
        instance.baseColor = (modelInstance.drawFlags & kModelDebugShowMeshletColors) != 0
            ? GetMeshletDebugColor(gModel.meshletIndexOffset + meshletIndex)
            : gModel.meshBaseColor * modelInstance.color;
        instance.uvScale = float2(1.0f, 1.0f);
        instance.uvOffset = float2(0.0f, 0.0f);
        instance.textureIndex = (modelInstance.drawFlags & kModelDebugShowMeshletColors) != 0
            ? 0xffffffffu
            : gModel.baseColorTextureIndex;
        instance.normalTextureIndex =
            (modelInstance.drawFlags &
                (kModelDebugShowMeshletColors | kModelDisableNormalMap)) != 0
            ? 0xffffffffu
            : gModel.normalTextureIndex;
        instance.normalScale = gModel.normalScale * modelInstance.normalScale;
        instance.materialFlags = (modelInstance.drawFlags & kModelUnlit) != 0
            ? kMaterialUnlit
            : 0u;
        instance.metallicRoughnessTextureIndex =
            (modelInstance.drawFlags & kModelDebugShowMeshletColors) != 0
            ? 0xffffffffu
            : gModel.metallicRoughnessTextureIndex;
        instance.occlusionTextureIndex =
            (modelInstance.drawFlags & kModelDebugShowMeshletColors) != 0
            ? 0xffffffffu
            : gModel.occlusionTextureIndex;
        const bool showMeshletColors =
            (modelInstance.drawFlags & kModelDebugShowMeshletColors) != 0;
        instance.metallic = showMeshletColors
            ? 0.0f
            : saturate(gModel.metallicFactor * modelInstance.metallicScale);
        instance.roughness = showMeshletColors
            ? 1.0f
            : saturate(gModel.roughnessFactor * modelInstance.roughnessScale);
        instance.occlusionStrength = saturate(
            gModel.occlusionStrength * modelInstance.occlusionStrength);
        instance.padding = float3(0.0f, 0.0f, 0.0f);

        outputVertices[localIndex] = MakeVertex(
            instance,
            gCamera,
            vertex.position,
            vertex.normal,
            vertex.uv);

        const float3 worldTangent = mul(
            float4(vertex.tangent.xyz, 0.0f),
            modelInstance.worldMatrix).xyz;
        const float worldTangentLengthSquared = dot(worldTangent, worldTangent);
        if (vertex.tangent.w != 0.0f && worldTangentLengthSquared > 1.0e-12f)
        {
            outputVertices[localIndex].tangent = float4(
                worldTangent * rsqrt(worldTangentLengthSquared),
                vertex.tangent.w * modelInstance.worldOrientationSign);
        }
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

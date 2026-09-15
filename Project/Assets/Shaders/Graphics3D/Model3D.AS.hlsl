#include "Graphics3D.hlsli"
#include "Model3D.hlsli"

ConstantBuffer<CameraData3D> gCullingCamera : register(b2);
ConstantBuffer<ModelDrawData> gCullingModel : register(b3);

groupshared ModelPayload gPayload;
groupshared uint gVisibleMeshletCount;

bool IsVisible(MeshletCullData cullData, ModelInstanceData instance)
{
    const float3 worldCenter = mul(
        float4(cullData.center, 1.0f),
        instance.worldMatrix).xyz;

    const float worldRadius = cullData.radius * instance.worldMaximumScale;

    [unroll]
    for (uint planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        const float signedDistance = dot(
            float4(worldCenter, 1.0f),
            gCullingCamera.frustumPlanes[planeIndex]);
        if (signedDistance < -worldRadius)
        {
            return false;
        }
    }

    return true;
}

[numthreads(kMeshletsPerAmplificationGroup, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint groupThreadIndex : SV_GroupIndex)
{
    if (groupThreadIndex == 0)
    {
        gVisibleMeshletCount = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    const uint64_t baseWorkIndex =
        uint64_t(gCullingModel.baseWorkIndexLow) |
        (uint64_t(gCullingModel.baseWorkIndexHigh) << 32);
    const uint64_t workIndex =
        baseWorkIndex +
        uint64_t(groupId.x) * kMeshletsPerAmplificationGroup +
        groupThreadIndex;
    const uint64_t workCount =
        uint64_t(gCullingModel.instanceCount) * gCullingModel.meshletCount;

    if (workIndex < workCount)
    {
        const uint localInstanceIndex = uint(workIndex / gCullingModel.meshletCount);
        const uint meshletIndex = uint(workIndex % gCullingModel.meshletCount);
        const uint instanceIndex = gCullingModel.instanceOffset + localInstanceIndex;

        StructuredBuffer<ModelInstanceData> instanceBuffer =
            ResourceDescriptorHeap[gCullingModel.instanceBufferIndex];
        StructuredBuffer<MeshletCullData> cullDataBuffer =
            ResourceDescriptorHeap[gCullingModel.meshletCullDataBufferIndex];
        if (IsVisible(cullDataBuffer[meshletIndex], instanceBuffer[instanceIndex]))
        {
            uint payloadIndex;
            InterlockedAdd(gVisibleMeshletCount, 1, payloadIndex);
            gPayload.visibleMeshlets[payloadIndex].instanceIndex = instanceIndex;
            gPayload.visibleMeshlets[payloadIndex].meshletIndex = meshletIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(gVisibleMeshletCount, 1, 1, gPayload);
}

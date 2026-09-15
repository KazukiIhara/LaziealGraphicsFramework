#include "Graphics3D.hlsli"
#include "Model3D.hlsli"

ConstantBuffer<CameraData3D> gCullingCamera : register(b2);
ConstantBuffer<ModelDrawData> gCullingModel : register(b3);

groupshared ModelPayload gPayload;
groupshared uint gVisibleMeshletCount;

bool IsVisible(MeshletCullData cullData)
{
    const float3 worldCenter = mul(
        float4(cullData.center, 1.0f),
        gCullingModel.worldMatrix).xyz;

    const float worldRadius = cullData.radius * gCullingModel.worldMaximumScale;

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

    const uint meshletIndex =
        groupId.x * kMeshletsPerAmplificationGroup + groupThreadIndex;
    if (meshletIndex < gCullingModel.meshletCount)
    {
        StructuredBuffer<MeshletCullData> cullDataBuffer =
            ResourceDescriptorHeap[gCullingModel.meshletCullDataBufferIndex];
        if (IsVisible(cullDataBuffer[meshletIndex]))
        {
            uint payloadIndex;
            InterlockedAdd(gVisibleMeshletCount, 1, payloadIndex);
            gPayload.meshletIndices[payloadIndex] = meshletIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(gVisibleMeshletCount, 1, 1, gPayload);
}

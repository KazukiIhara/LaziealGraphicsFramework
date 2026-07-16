#include "Graphics3D.hlsli"

struct LineData
{
    InstanceData3D instData;
    float3 start;
    float padding0;
    float3 end;
    float padding1;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<LineData> gLine : register(t0);

[outputtopology("line")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput3D vertices[2],
    out indices uint2 lines[1])
{
    SetMeshOutputCounts(2, 1);

    LineData lineData = gLine[DTid.x];

    float4 startWorldPos = mul(float4(lineData.start, 1.0f), lineData.instData.worldMatrix);
    float4 endWorldPos = mul(float4(lineData.end, 1.0f), lineData.instData.worldMatrix);

    vertices[0].position = mul(startWorldPos, gCamera.viewProj);
    vertices[0].baseColor = lineData.instData.baseColor;
    vertices[0].normal = ToWorldNormal(lineData.instData, float3(0.0f, 0.0f, 1.0f));

    vertices[1].position = mul(endWorldPos, gCamera.viewProj);
    vertices[1].baseColor = lineData.instData.baseColor;
    vertices[1].normal = ToWorldNormal(lineData.instData, float3(0.0f, 0.0f, 1.0f));

    lines[0] = uint2(0, 1);
}

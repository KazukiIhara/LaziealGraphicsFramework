#include "Graphics3D.hlsli"

static const uint kSegments = 32;
static const uint kRingCount = 3;
static const uint kVertexCount = kSegments * kRingCount;
static const uint kLineCount = kSegments * kRingCount;
static const float kPI = 3.14159265358979323846f;

struct MeshletBoundsData
{
    InstanceData3D instance;
    float radius;
    float3 padding;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<MeshletBoundsData> gMeshletBounds : register(t0);

float3 MakeRingPosition(uint ringIndex, float angle)
{
    const float cosine = cos(angle);
    const float sine = sin(angle);

    if (ringIndex == 0)
    {
        return float3(cosine, sine, 0.0f);
    }
    if (ringIndex == 1)
    {
        return float3(cosine, 0.0f, sine);
    }
    return float3(0.0f, cosine, sine);
}

[outputtopology("line")]
[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadId : SV_DispatchThreadID,
    out vertices MSOutput3D outputVertices[kVertexCount],
    out indices uint2 outputLines[kLineCount])
{
    SetMeshOutputCounts(kVertexCount, kLineCount);

    const MeshletBoundsData bounds = gMeshletBounds[dispatchThreadId.x];

    [unroll]
    for (uint ringIndex = 0; ringIndex < kRingCount; ++ringIndex)
    {
        const uint firstVertex = ringIndex * kSegments;
        [unroll]
        for (uint segmentIndex = 0; segmentIndex < kSegments; ++segmentIndex)
        {
            const float angle =
                float(segmentIndex) / float(kSegments) * (2.0f * kPI);
            const float3 localPosition = MakeRingPosition(ringIndex, angle) * bounds.radius;

            outputVertices[firstVertex + segmentIndex] = MakeVertex(
                bounds.instance,
                gCamera,
                localPosition,
                normalize(localPosition),
                float2(0.0f, 0.0f));

            outputLines[firstVertex + segmentIndex] = uint2(
                firstVertex + segmentIndex,
                firstVertex + ((segmentIndex + 1) % kSegments));
        }
    }
}

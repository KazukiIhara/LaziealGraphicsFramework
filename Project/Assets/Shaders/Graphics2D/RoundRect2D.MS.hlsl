#include "Graphics2D.hlsli"

static const uint kCornerSegmentCount = 8;
static const uint kCornerCount = 4;
static const uint kRoundRectVertexCount = kCornerCount * (kCornerSegmentCount + 1);
static const float kPi = 3.14159265358979323846f;

struct RoundRectData2D
{
    InstanceData2D instData;
    float2 leftTop;
    float2 rightBottom;
    float2 radius;
};

StructuredBuffer<RoundRectData2D> gRoundRect : register(t0);

float2 GetCornerCenter(RoundRectData2D rect, uint cornerIndex)
{
    const float left = rect.leftTop.x;
    const float top = rect.leftTop.y;
    const float right = rect.rightBottom.x;
    const float bottom = rect.rightBottom.y;

    if (cornerIndex == 0)
    {
        return float2(right - rect.radius.x, top - rect.radius.y);
    }
    if (cornerIndex == 1)
    {
        return float2(left + rect.radius.x, top - rect.radius.y);
    }
    if (cornerIndex == 2)
    {
        return float2(left + rect.radius.x, bottom + rect.radius.y);
    }

    return float2(right - rect.radius.x, bottom + rect.radius.y);
}

float GetCornerStartAngle(uint cornerIndex)
{
    if (cornerIndex == 0)
    {
        return 0.0f;
    }
    if (cornerIndex == 1)
    {
        return kPi * 0.5f;
    }
    if (cornerIndex == 2)
    {
        return kPi;
    }

    return kPi * 1.5f;
}

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput2D vertices[kRoundRectVertexCount + 1],
    out indices uint3 triangles[kRoundRectVertexCount])
{
    // 角丸四角形1枚分を出力
    SetMeshOutputCounts(kRoundRectVertexCount + 1, kRoundRectVertexCount);

    // インデックスから角丸四角形データを取得
    RoundRectData2D rect = gRoundRect[DTid.x];

    const float2 center = (rect.leftTop + rect.rightBottom) * 0.5f;
    vertices[0].position = float4(center, 0.0f, 1.0f);
    vertices[0].baseColor = rect.instData.baseColor;

    for (uint cornerIndex = 0; cornerIndex < kCornerCount; ++cornerIndex)
    {
        const float2 cornerCenter = GetCornerCenter(rect, cornerIndex);
        const float startAngle = GetCornerStartAngle(cornerIndex);

        for (uint segmentIndex = 0; segmentIndex <= kCornerSegmentCount; ++segmentIndex)
        {
            const uint vertexIndex = cornerIndex * (kCornerSegmentCount + 1) + segmentIndex + 1;
            const float angle = startAngle + (float(segmentIndex) / float(kCornerSegmentCount)) * kPi * 0.5f;
            const float2 offset = float2(cos(angle) * rect.radius.x, sin(angle) * rect.radius.y);

            vertices[vertexIndex].position = float4(cornerCenter + offset, 0.0f, 1.0f);
            vertices[vertexIndex].baseColor = rect.instData.baseColor;
        }
    }

    for (uint i = 0; i < kRoundRectVertexCount; ++i)
    {
        const uint nextIndex = ((i + 1) % kRoundRectVertexCount) + 1;
        triangles[i] = uint3(0, i + 1, nextIndex);
    }
}

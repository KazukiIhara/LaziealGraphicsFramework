#include "Graphics2D.hlsli"

static const uint kCircleSegmentCount = 64;
static const float kPi = 3.14159265358979323846f;

struct CircleData2D
{
    InstanceData2D instData;
    float2 center;
    float2 radius;
};

StructuredBuffer<CircleData2D> gCircle : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput2D vertices[kCircleSegmentCount + 2],
    out indices uint3 triangles[kCircleSegmentCount])
{
    // 円1個分を出力
    SetMeshOutputCounts(kCircleSegmentCount + 2, kCircleSegmentCount);

    // インデックスから円データを取得
    CircleData2D circle = gCircle[DTid.x];

    vertices[0] = MakeVertex2D(
        circle.center,
        circle.instData,
        float2(0.5f, 0.5f));

    for (uint i = 0; i <= kCircleSegmentCount; ++i)
    {
        const float angle = (float(i) / float(kCircleSegmentCount)) * 2.0f * kPi;
        const float2 offset = float2(cos(angle) * circle.radius.x, sin(angle) * circle.radius.y);
        const float2 safeRadius = max(abs(circle.radius), float2(1.0e-6f, 1.0e-6f));

        const float2 uv = float2(
            offset.x / safeRadius.x * 0.5f + 0.5f,
            0.5f - offset.y / safeRadius.y * 0.5f);
        vertices[i + 1] = MakeVertex2D(circle.center + offset, circle.instData, uv);
    }

    for (uint i = 0; i < kCircleSegmentCount; ++i)
    {
        triangles[i] = uint3(0, i + 1, i + 2);
    }
}

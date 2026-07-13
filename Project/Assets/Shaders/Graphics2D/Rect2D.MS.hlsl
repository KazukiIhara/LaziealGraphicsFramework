#include "Graphics2D.hlsli"

struct RectData2D
{
    InstanceData2D instData;
    float2 vertices[4];
};

StructuredBuffer<RectData2D> gRect : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput2D vertices[4],
    out indices uint3 triangles[2])
{
    // 四角形1枚分を出力
    SetMeshOutputCounts(4, 2);

    // インデックスから四角形データを取得
    RectData2D rect = gRect[DTid.x];

    for (uint i = 0; i < 4; ++i)
    {
        // 頂点をセット
        vertices[i].position = float4(rect.vertices[i].xy, 0.0f, 1.0f);

        // 色をセット
        vertices[i].baseColor = rect.instData.baseColor;
    }

    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(2, 1, 3);
}

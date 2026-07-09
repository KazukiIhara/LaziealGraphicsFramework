#include "Graphics2D.hlsli"

struct TriangleData2D
{
    InstanceData2D instData;
    float2 vertexOffsets[3];
};

ConstantBuffer<CameraData2D> gCamera : register(b0);
StructuredBuffer<TriangleData2D> gTriangle : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    uint3 DTid : SV_DispatchThreadID,
    out vertices MSOutput2D vertices[3],
    out indices uint3 triangles[1])
{
    // 三角形1枚分を出力
    SetMeshOutputCounts(3, 1);

    // インデックスから三角形データを取得
    TriangleData2D tri = gTriangle[DTid.x];

    for (uint i = 0; i < 3; ++i)
    {
        // 頂点を変換
        float4 localPos = float4(tri.vertexOffsets[i].xy, 0.0f, 1.0f);
        float4 worldPos = mul(localPos, tri.instData.worldMatrix);
        vertices[i].position = mul(worldPos, gCamera.viewProj);

        // 色をセット
        vertices[i].baseColor = tri.instData.baseColor;
    }

    triangles[0] = uint3(0, 1, 2);
}
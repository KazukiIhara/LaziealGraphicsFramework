#include "Graphics3D.hlsli"
#include "Font3D.hlsli"

struct FontGlyphData
{
    float3 origin;
    float padding0;
    float2 offset;
    float2 size;
    float2 uvScale;
    float2 uvOffset;
    float3 right;
    uint textureIndex;
    float3 up;
    uint billboard;
    float4 color;
};

ConstantBuffer<CameraData3D> gCamera : register(b0);
StructuredBuffer<FontGlyphData> gGlyph : register(t0);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID,
    out vertices FontGlyphOutput vertices[4],
    out indices uint3 triangles[2])
{
    SetMeshOutputCounts(4, 2);
    FontGlyphData glyph = gGlyph[id.x];

    float3 right = glyph.right;
    float3 up = glyph.up;
    if (glyph.billboard != 0)
    {
        right = normalize(mul(float4(1, 0, 0, 0), gCamera.inverseViewProj).xyz);
        up = normalize(mul(float4(0, 1, 0, 0), gCamera.inverseViewProj).xyz);
    }

    for (uint i = 0; i < 4; ++i)
    {
        float2 corner = float2(i % 2, i / 2);
        float2 localPosition = glyph.offset + corner * glyph.size;
        float3 worldPosition = glyph.origin
            + right * localPosition.x
            - up * localPosition.y;

        vertices[i].position = mul(float4(worldPosition, 1), gCamera.viewProj);
        vertices[i].color = glyph.color;
        vertices[i].uv = corner * glyph.uvScale + glyph.uvOffset;
        vertices[i].textureIndex = glyph.textureIndex;
    }

    triangles[0] = uint3(0, 1, 2);
    triangles[1] = uint3(1, 3, 2);
}

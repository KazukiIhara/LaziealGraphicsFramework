// インスタンシング描画用のデータ
struct InstanceData2D
{
    float4 baseColor;
    float2 uvScale;
    float2 uvOffset;
    uint textureIndex;
};

// MSからPSに渡す構造体
struct MSOutput2D
{
    float4 position : SV_Position;
    float4 baseColor : COLOR0;
    float2 uv : TEXCOORD0;
    nointerpolation uint textureIndex : TEXCOORD1;
};

MSOutput2D MakeVertex2D(float2 position, InstanceData2D instance, float2 uv)
{
    MSOutput2D output;
    output.position = float4(position, 0.0f, 1.0f);
    output.baseColor = instance.baseColor;
    output.uv = uv * instance.uvScale + instance.uvOffset;
    output.textureIndex = instance.textureIndex;
    return output;
}

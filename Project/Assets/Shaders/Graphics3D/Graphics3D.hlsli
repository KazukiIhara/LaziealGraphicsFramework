
// インスタンシング描画用のデータ
struct InstanceData3D
{
    float4x4 worldMatrix;
    float4x4 worldInverseTranspose;
    float4 baseColor;
};

// カメラデータ
struct CameraData3D
{
    float4x4 viewProj;
    float3 worldPos;
    float padding;
};

// MSからPSに渡す構造体
struct MSOutput3D
{
    float4 position : SV_Position;
    float4 baseColor : COLOR0;
    float3 normal : NORMAL0;
};

float3 ToWorldNormal(InstanceData3D instData, float3 localNormal)
{
    return normalize(mul(float4(localNormal, 0.0f), instData.worldInverseTranspose).xyz);
}

float4 ToNormalColor(float3 normal)
{
    float3 normalizedNormal = normalize(normal);
    return float4(normalizedNormal * 0.5f + 0.5f, 1.0f);
}

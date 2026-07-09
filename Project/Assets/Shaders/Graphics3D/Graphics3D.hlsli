
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
};
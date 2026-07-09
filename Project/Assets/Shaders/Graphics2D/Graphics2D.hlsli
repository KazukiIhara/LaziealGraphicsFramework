
// インスタンシング描画用のデータ
struct InstanceData2D
{
    float4x4 worldMatrix;
    float4 baseColor;
};

// カメラデータ
struct CameraData2D
{
    float4x4 viewProj;
};

// MSからPSに渡す構造体
struct MSOutput2D
{
    float4 position : SV_Position;
    float4 baseColor : COLOR0;
};
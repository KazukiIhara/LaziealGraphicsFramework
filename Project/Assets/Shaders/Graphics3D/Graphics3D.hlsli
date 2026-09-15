
// インスタンシング描画用のデータ
struct InstanceData3D
{
    float4x4 worldMatrix;
    float4x4 worldInverseTranspose;
    float4 baseColor;
    float2 uvScale;
    float2 uvOffset;
    uint textureIndex;
    uint3 padding;
};

// カメラデータ
struct CameraData3D
{
    float4x4 viewProj;
    float3 worldPos;
    float padding;
    float4 frustumPlanes[6];
};

// MSからPSに渡す構造体
struct MSOutput3D
{
    float4 position : SV_Position;
    float4 baseColor : COLOR0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    nointerpolation uint textureIndex : TEXCOORD1;
};

MSOutput3D MakeVertex(InstanceData3D instance, CameraData3D camera,
    float3 position, float3 normal, float2 uv)
{
    MSOutput3D output;
    output.position = mul(mul(float4(position, 1.0f), instance.worldMatrix), camera.viewProj);
    output.baseColor = instance.baseColor;
    output.normal = normalize(mul(float4(normal, 0.0f), instance.worldInverseTranspose).xyz);
    output.uv = uv * instance.uvScale + instance.uvOffset;
    output.textureIndex = instance.textureIndex;
    return output;
}

float3 ToWorldNormal(InstanceData3D instData, float3 localNormal)
{
    return normalize(mul(float4(localNormal, 0.0f), instData.worldInverseTranspose).xyz);
}

float4 ToNormalColor(float3 normal)
{
    float3 normalizedNormal = normalize(normal);
    return float4(normalizedNormal * 0.5f + 0.5f, 1.0f);
}

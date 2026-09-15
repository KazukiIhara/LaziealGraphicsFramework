
static const uint kMaterialUnlit = 1u << 0;

// インスタンシング描画用のデータ
struct InstanceData3D
{
    float4x4 worldMatrix;
    float4x4 worldInverseTranspose;
    float4 baseColor;
    float2 uvScale;
    float2 uvOffset;
    uint textureIndex;
    uint normalTextureIndex;
    float normalScale;
    uint materialFlags;
    uint metallicRoughnessTextureIndex;
    uint occlusionTextureIndex;
    float metallic;
    float roughness;
    float occlusionStrength;
    float3 padding;
};

// カメラデータ
struct CameraData3D
{
    float4x4 viewProj;
    float4x4 inverseViewProj;
    float3 worldPos;
    float padding;
    float4 frustumPlanes[6];
};

float GetUnlitFlag(InstanceData3D instance)
{
    return (instance.materialFlags & kMaterialUnlit) != 0 ? 1.0f : 0.0f;
}

// MSからPSに渡す構造体
struct MSOutput3D
{
    float4 position : SV_Position;
    float4 baseColor : COLOR0;
    float4 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    nointerpolation uint textureIndex : TEXCOORD1;
    nointerpolation uint normalTextureIndex : TEXCOORD2;
    nointerpolation float normalScale : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
    float4 tangent : TANGENT0;
    nointerpolation uint2 pbrTextureIndices : TEXCOORD5;
    nointerpolation float3 pbrFactors : TEXCOORD6;
};

MSOutput3D MakeVertex(InstanceData3D instance, CameraData3D camera,
    float3 position, float3 normal, float2 uv)
{
    MSOutput3D output;
    float4 worldPosition = mul(float4(position, 1.0f), instance.worldMatrix);
    output.position = mul(worldPosition, camera.viewProj);
    output.baseColor = instance.baseColor;
    output.normal = float4(
        normalize(mul(float4(normal, 0.0f), instance.worldInverseTranspose).xyz),
        GetUnlitFlag(instance));
    output.uv = uv * instance.uvScale + instance.uvOffset;
    output.textureIndex = instance.textureIndex;
    output.normalTextureIndex = instance.normalTextureIndex;
    output.normalScale = instance.normalScale;
    output.worldPosition = worldPosition.xyz;
    output.tangent = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.pbrTextureIndices = uint2(
        instance.metallicRoughnessTextureIndex,
        instance.occlusionTextureIndex);
    output.pbrFactors = float3(
        instance.metallic,
        instance.roughness,
        instance.occlusionStrength);
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

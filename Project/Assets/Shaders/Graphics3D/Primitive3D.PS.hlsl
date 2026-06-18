#include "Graphics3D.hlsli"

float4 main(MSOutput input) : SV_Target
{
    float4 output;
        
    output = input.baseColor;
    
    return output;
}
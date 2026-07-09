#include "Graphics3D.hlsli"

float4 main(MSOutput3D input) : SV_Target
{
    float4 output;
        
    output = input.baseColor;
    
    return output;
}
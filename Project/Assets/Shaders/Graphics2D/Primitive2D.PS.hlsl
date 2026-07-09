#include "Graphics2D.hlsli"

float4 main(MSOutput2D input) : SV_Target
{
    float4 output;

    output = input.baseColor;

    return output;
}
#include "Common.hlsl"

struct VSInput
{
    float3 positionL : POSITION;
    float3 normalL : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float3 positionL : POSITION;
    float4 positionH : SV_POSITION;
};

PSInput main(VSInput input)
{
    PSInput output;
    
    // look-up vector for cubemap sampling
    output.positionL = input.positionL;
    
    float4 positionW = mul(float4(output.positionL, 1.f), world);
    
    // Always center sky about camera
    positionW.xyz += eyeWorldPosition;
    
    //  Set z = w so that z/w = 1 (i.e., skydome always on far plane)
    output.positionH = mul(positionW, viewProjection).xyww;
    
    return output;
}
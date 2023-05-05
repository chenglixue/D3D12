#include "Common.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float3 positionW : POSITION;
    float4 positionH : SV_POSITION;
    float3 normalW : NORMAL;
    float2 uv : TEXCOORD;
};

PSInput main(VSInput input)
{
    PSInput result;

    float4 positionW = mul(float4(input.position, 1.f), world);
    
    result.positionW = positionW.xyz;
    
    result.positionH = mul(positionW, viewProjection);
    
    result.normalW = mul(input.normal, (float3x3)world);

    result.uv = input.uv;

	return result;
}
#pragma once

#include "Common.hlsl"

static float4x4 scale =
{
    1.2f, 0.f, 0.f, 0.f,
	0.f, 1.2f, 0.f, 0.f,
	0.f, 0.f, 1.2f, 0.f,
	0.f, 0.f, 0.f, 1.f
};

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
    PSInput output;
    
    float4 positionW = mul(mul(float4(input.position, 1.f), world), scale);
    
    output.positionW = positionW.xyz;
    output.normalW = mul(mul(input.normal, (float3x3) world), (float3x3)scale);
    
    output.positionH = mul(positionW, viewProjection);
    
    output.uv = input.uv;
    
	return output;
}
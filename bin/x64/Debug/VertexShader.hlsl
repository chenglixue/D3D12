

cbuffer ObjectCB : register(b0)
{
    float4x4 world;
}

cbuffer PassCB : register(b1)
{
    float4x4 view;
    float4x4 inverseView;
    float4x4 projection;
    float4x4 inverseProjection;
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    
    float3 eyeWorldPosition;
    
    float pad;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    //float3 tangent : TANGENT;
    //float3 bitNormal : BINORMAL;
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
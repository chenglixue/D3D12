struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ObjectCB : register(b1)
{
    float4x4 world;
}

cbuffer PassCB : register(b0)
{
    float4x4 view;
    float4x4 inverseView;
    float4x4 projection;
    float4x4 inverseProjection;
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
}

PSInput VSMain(VSInput input)
{
    PSInput result;

    float4 posWorld = mul(float4(input.position, 1.0f), world);
    result.position = mul(posWorld, viewProjection);
    result.color = float4(1,1,1,0);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}

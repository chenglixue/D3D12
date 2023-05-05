//-------------------------------------------------------------------------- macro --------------------------------------------------------------------------

#ifndef DIRECTION_LIGHT_COUNT
#define DIRECTION_LIGHT_COUNT 1
#endif

#ifndef POINT_LIGHT_COUNT
#define POINT_LIGHT_COUNT 1
#endif

#ifndef SPOT_LIGHT_COUNT
#define SPOT_LIGHT_COUNT 1
#endif

#include "shading.hlsl"

//-------------------------------------------------------------------------- declaration --------------------------------------------------------------------------
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

    Light lights[MAX_LIGHT_COUNT];
}

cbuffer MaterialCB : register(b2)
{
    int specualrShiness;
    float ambientAlbedo;
}

SamplerState g_SamperPointWrap : register(s0);
SamplerState g_SamperPointClamp : register(s1);
SamplerState g_SamperLinerWrap : register(s2);
SamplerState g_SamperLinerClamp : register(s3);
SamplerState g_SamperAnisotropyWrap : register(s4);
SamplerState g_SamperAnisotropyClamp : register(s5);

Texture2D g_diffuseTexture : register(t0, space0);
Texture2D g_specularTexture : register(t1, space0);
TextureCube g_cubemap : register(t2, space0);
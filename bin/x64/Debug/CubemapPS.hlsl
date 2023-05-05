#include "CubemapVS.hlsl"

float4 main(PSInput input) : SV_TARGET
{
    return g_cubemap.Sample(g_SamperLinerWrap, input.positionL);
}


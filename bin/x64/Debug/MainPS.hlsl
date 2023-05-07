//-------------------------------------------------------------------------- macro --------------------------------------------------------------------------
#include "MainVS.hlsl"

//-------------------------------------------------------------------------- main function --------------------------------------------------------------------------

float4 main(PSInput input) : SV_TARGET
{
    float4 textureDiffuseAlbedo = g_diffuseTexture.Sample(g_SamperAnisotropyWrap, input.uv);
    float4 textureSpecularAlbedo = g_specularTexture.Sample(g_SamperAnisotropyWrap, input.uv);
    
    input.normalW = normalize(input.normalW);
    
    float3 toEyeDirW = eyeWorldPosition - input.positionW;
    float toEyeLength = length(toEyeDirW);
    toEyeDirW = normalize(toEyeDirW);
    
    Material material = { textureDiffuseAlbedo, textureSpecularAlbedo, ambientAlbedo, specualrShiness };

    float4 resultLightColor = CalcLightColor(lights, material, input.normalW, toEyeDirW, input.positionW);
    
    resultLightColor.a = textureDiffuseAlbedo.a;
    
    return resultLightColor;
}
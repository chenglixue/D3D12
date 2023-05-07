//-------------------------------------------------------------------------- macro --------------------------------------------------------------------------
#include "MainVS.hlsl"

//-------------------------------------------------------------------------- main function --------------------------------------------------------------------------

float4 main(PSInput input) : SV_TARGET
{
    float4 textureDiffuseAlbedo = g_diffuseTexture.Sample(g_SamperAnisotropyWrap, input.uv);
    float4 textureSpecularAlbedo = g_specularTexture.Sample(g_SamperAnisotropyWrap, input.uv);
    float4 normalMapSample = g_normalmapTexture.Sample(g_SamperAnisotropyWrap, input.uv);
    float3 averageNormalW = normalmapToWolrd(normalMapSample.rgb, input.normalW, input.tangentW);
    
    input.normalW = normalize(input.normalW);
    
    float3 toEyeDirW = eyeWorldPosition - averageNormalW;
    float toEyeLength = length(toEyeDirW);
    toEyeDirW = normalize(toEyeDirW);
    
    Material material = { textureDiffuseAlbedo, textureSpecularAlbedo, ambientAlbedo, specualrShiness };

    float4 resultLightColor = CalcLightColor(lights, material, averageNormalW, toEyeDirW, input.positionW);
    
    resultLightColor.a = textureDiffuseAlbedo.a;
    
    return resultLightColor;
}

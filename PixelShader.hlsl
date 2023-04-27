//-------------------------------------------------------------------------- macro --------------------------------------------------------------------------
#define MAX_LIGHT_COUNT 3

#ifndef DIRECTION_LIGHT_COUNT
	#define DIRECTION_LIGHT_COUNT 0
#endif

#ifndef POINT_LIGHT_COUNT
	#define POINT_LIGHT_COUNT 0
#endif

#ifndef SPOT_LIGHT_COUNT
	#define SPOT_LIGHT_COUNT 0
#endif

//-------------------------------------------------------------------------- declaration --------------------------------------------------------------------------

struct PSInput
{
    float3 positionW : POSITION;
    float4 positionH : SV_POSITION;
    float3 normalW : NROMAL0;
    float2 uv : TEXCOORD0;
};

struct Material
{
    float ambientAlbedo;

    float4 diffuseAlbedo;

    int specualrShiness;
};

struct Light
{
	// light color
    float3 lightColor;

	// for falloff function
    float falloffStart;

	// light direction
    float3 lightDirection;

	// for falloff function
    float falloffEnd;

	// light position
    float3 lightPosition;

	// spot equation's pow
    float spotPower;
};

float3 BlinnPhong(float3 lightColor, float3 toLightDir, float3 normal, float3 toEyeDir, Material material);

float CalcAttenuation(float falloffStart, float falloffEnd, float d);

float3 CalcDiretionLight(Light light, Material material, float3 normal, float3 toEyeDir);

float3 CalcPointLight(Light light, Material material, float3 normal, float3 toEyeDir, float3 objectPos);

float3 CaleSpotLight(Light light, Material material, float3 normal, float3 toEyeDir, float3 objectPos);

float4 CalcLightColor(Light lights[MAX_LIGHT_COUNT], Material material, float3 normal, float3 toEyeDir, float3 objectPos);

//-------------------------------------------------------------------------- binding data --------------------------------------------------------------------------

cbuffer MaterialCB : register(b2)
{
    float4 diffuseAlbedo;
    int specualrShiness;
    float ambientAlbedo;
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

SamplerState gSamperPointWrap           : register(s0);
SamplerState gSamperPointClamp          : register(s1);
SamplerState gSamperLinerWrap           : register(s2);
SamplerState gSamperLinerClamp          : register(s3);
SamplerState gSamperAnisotropyWrap      : register(s4);
SamplerState gSamperAnisotropyClamp     : register(s5);

Texture2D gTexture : register(t0);

//-------------------------------------------------------------------------- main function --------------------------------------------------------------------------

float4 main(PSInput input) : SV_TARGET
{
    float4 textureDiffuseAlbedo = gTexture.Sample(gSamperAnisotropyWrap, input.uv) * diffuseAlbedo;
    
    input.normalW = normalize(input.normalW);
    
    float3 toEyeDirW = normalize(eyeWorldPosition - input.positionW);
    
    Material material = { ambientAlbedo, textureDiffuseAlbedo, specualrShiness };

    float4 resultLightColor = CalcLightColor(lights, material, input.normalW, toEyeDirW, input.positionW);
    
    resultLightColor.a = textureDiffuseAlbedo.a;
    
    return resultLightColor;
}


//-------------------------------------------------------------------------- function define --------------------------------------------------------------------------

float3 BlinnPhong(float3 lightColor, float3 toLightDir, float3 normal, float3 toEyeDir, Material material)
{
	// ambient
    float3 ambient = lightColor * float3(material.ambientAlbedo, material.ambientAlbedo, material.ambientAlbedo);

    float3 diffuse = material.diffuseAlbedo.rgb * max(0.f, dot(normal, toLightDir)) * lightColor;

	// specular reflection
    float3 halfVector = normalize(toLightDir + toEyeDir);
    float3 specular = pow(max(0, dot(halfVector, normal)), material.specualrShiness) * lightColor;

    return ambient + diffuse + specular;
}

// calculate falloff of light strength
float CalcAttenuation(float falloffStart, float falloffEnd, float d)
{
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// calculate direction light
float3 CalcDiretionLight(Light light, Material material, float3 normal, float3 toEyeDir)
{
    float3 toLightDir = -light.lightDirection;

    float3 lightColor = max(dot(normal, toLightDir), 0) * light.lightColor;

    return BlinnPhong(lightColor, toLightDir, normal, toEyeDir, material);
}

// calculate point light
float3 CalcPointLight(Light light, Material material, float3 normal, float3 toEyeDir, float3 objectPos)
{
    float3 toLightDir = light.lightPosition - objectPos;

	// d of falloff equation
    float d = length(toLightDir);

    if (d > light.falloffEnd)
    {
        return float3(0.f, 0.f, 0.f);
    }
	
	// normalize
    toLightDir /= d;
	
	// Lambert
    float3 lightColor = max(dot(toLightDir, normal), 0.f) * light.lightColor;
	
	// calc attenuation accroding to distance
    lightColor *= CalcAttenuation(light.falloffStart, light.falloffEnd, d);
	
    return BlinnPhong(lightColor, toLightDir, normal, toEyeDir, material);
}

// calculate spot light
float3 CaleSpotLight(Light light, Material material, float3 normal, float3 toEyeDir, float3 objectPos)
{
    float3 toLightDir = light.lightPosition - objectPos;

	// d of falloff equation
    float d = length(toLightDir);

    if (d > light.falloffEnd)
    {
        return float3(0.f, 0.f, 0.f);
    }
	
	// normalize
    toLightDir /= d;
	
	// Lambert
    float3 lightColor = max(dot(toLightDir, normal), 0.f) * light.lightColor;
	
	// calc attenuation accroding to distance
    lightColor *= CalcAttenuation(light.falloffStart, light.falloffEnd, d);
	
	// scale spot light strength
    float spotLightFactor = pow(max(0, dot(toLightDir, normal)), light.spotPower);
    lightColor *= spotLightFactor;

    return BlinnPhong(lightColor, toLightDir, normal, toEyeDir, material);
}

float4 CalcLightColor(Light lights[MAX_LIGHT_COUNT], Material material, float3 normal, float3 toEyeDir, float3 objectPos)
{
    float3 result = 0.f;
	
    int i = 0;
	
#if (DIRECTION_LIGHT_COUNT > 0)

    for (i = 0; i < DIRECTION_LIGHT_COUNT; ++i)
    {
        result += CalcDiretionLight(lights[i], material, normal, toEyeDir);
    }
	
#endif
	
#if (POINT_LIGHT_COUNT > 0)
	
    for (i = DIRECTION_LIGHT_COUNT; i < DIRECTION_LIGHT_COUNT + POINT_LIGHT_COUNT; ++i)
    {
        result += CalcPointLight(lights[i], material, normal, toEyeDir, objectPos);
    }
	
#endif
	
#if (SPOT_LIGHT_COUNT > 0)
	
    for (i = DIRECTION_LIGHT_COUNT + POINT_LIGHT_COUNT; i < DIRECTION_LIGHT_COUNT + POINT_LIGHT_COUNT + SPOT_LIGHT_COUNT; ++i)
    {
        result += CaleSpotLight(lights[i], material, normal, toEyeDir, objectPos);
    }
	
#endif

    return float4(result, 0.f);
}
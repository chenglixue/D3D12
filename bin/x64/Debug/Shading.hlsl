#define MAX_LIGHT_COUNT 3

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

struct Material
{
    float4 diffuseAlbedo;
    
    float4 specularAlbedo;
    
    float ambientAlbedo;

    int specualrShiness;
};

float3 BlinnPhong(float3 lightColor, float3 toLightDir, float3 normal, float3 toEyeDir, Material material)
{
	// ambient
    float3 ambient = lightColor * material.ambientAlbedo * material.diffuseAlbedo.rgb;

    float3 diffuse = material.diffuseAlbedo.rgb * max(0.f, dot(normal, toLightDir)) * lightColor;

	// specular reflection
    float3 halfVector = normalize(toLightDir + toEyeDir);
    float3 specular = pow(max(0, dot(halfVector, normal)), material.specualrShiness) * lightColor * material.specularAlbedo.rgb;

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
    float3 toLightDir = normalize(-light.lightDirection);

    return BlinnPhong(light.lightColor, toLightDir, normal, toEyeDir, material);
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
	
	// calc attenuation accroding to distance
    light.lightColor *= CalcAttenuation(light.falloffStart, light.falloffEnd, d);
	
    return BlinnPhong(light.lightColor, toLightDir, normal, toEyeDir, material);
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
	
	// calc attenuation accroding to distance
    light.lightColor *= CalcAttenuation(light.falloffStart, light.falloffEnd, d);
	
	// scale spot light strength
    float spotLightFactor = pow(max(0, dot(toLightDir, normal)), light.spotPower);
    light.lightColor *= spotLightFactor;

    return BlinnPhong(light.lightColor, toLightDir, normal, toEyeDir, material);
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

    return float4(result, 1.f);
}
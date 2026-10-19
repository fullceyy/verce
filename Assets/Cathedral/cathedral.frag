#version 460 core

struct PointLight
{
    vec3 v3LightPosition;
    vec3 v3LightIntensity;
    vec3 v3Falloff;
};

#define MAX_POINT_LIGHTS 16

layout(binding = 1) uniform CameraData
{
    mat4 m4ViewProjection;
    vec3 v3CameraPosition;
};

layout(binding = 2) uniform PointLightData
{
    PointLight PointLights[MAX_POINT_LIGHTS];
};

uniform int iNumPointLights;

uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;
uniform sampler2D uRoughnessMap;

const float M_PI     = 3.14159265358979323846;
const float M_RCPPI  = 0.31830988618379067154;

in vec3 v3VertexPosIn;
in vec3 v3VertexNormalIn;
in vec2 v2VertexUVIn;

out vec3 v3ColorOut;

// Light falloff
vec3 lightFalloff(in vec3 v3LightIntensity, in vec3 v3Falloff, in vec3 v3LightPosition, in vec3 v3Position)
{
    float fDistance = distance(v3LightPosition, v3Position);
    float fFalloff = v3Falloff.x + (v3Falloff.y * fDistance) + (v3Falloff.z * fDistance * fDistance);
    return v3LightIntensity / fFalloff;
}

// Schlick Fresnel approximation
vec3 schlickFresnel(in vec3 v3ViewDirection, in vec3 v3HalfVector, in vec3 v3SpecularColour)
{
    float fVH = max(dot(v3ViewDirection, v3HalfVector), 0.0);
    return v3SpecularColour + (1.0 - v3SpecularColour) * pow(1.0 - fVH, 5.0);
}


// Trowbridge-Reitz / GGX normal distribution function
float TRDistribution(in vec3 v3Normal, in vec3 v3HalfVector, in float fRoughness)
{
    float fRoughnessSq = fRoughness * fRoughness;
    float fNH = max(dot(v3Normal, v3HalfVector),0.0);
    float fDenominator = fNH * fNH * (fRoughnessSq - 1.0) + 1.0;
    return fRoughnessSq / (M_PI * fDenominator * fDenominator);
}


// GGX visibility function
float GGXVisibility(in vec3 v3Normal, in vec3 v3LightDirection, in vec3 v3ViewDirection, in float fRoughness)
{
    float fNL = max(dot(v3Normal, v3LightDirection),0.0);
    float fNV = max(dot(v3Normal, v3ViewDirection),0.0);
    float fRoughnessSq = fRoughness * fRoughness;
    float fRoughnessMod = 1.0 - fRoughnessSq;
    float fRecipG1 = fNL + sqrt(fRoughnessSq + fRoughnessMod * fNL * fNL);
    float fRecipG2 = fNV + sqrt( fRoughnessSq +fRoughnessMod * fNV * fNV);
    return 1.0 / (fRecipG1 * fRecipG2);
}


// GGX BRDF
vec3 GGX(in vec3 v3Normal,
    in vec3 v3LightDirection,
    in vec3 v3ViewDirection,
    in vec3 v3LightIrradiance,
    in vec3 v3DiffuseColour,
    in vec3 v3SpecularColour,
    in float fRoughness)
{
    // Lambertian diffuse component
    vec3 v3Diffuse = v3DiffuseColour * M_RCPPI;
    // Calculate half-vector
    vec3 v3HalfVector = normalize(v3ViewDirection + v3LightDirection);
    // Calculate Fresnel
    vec3 v3F = schlickFresnel(v3ViewDirection, v3HalfVector, v3SpecularColour);
    // Calculate normal distribution
    float fD = TRDistribution(v3Normal, v3HalfVector, fRoughness);
    // Calculate visibility
    float fV = GGXVisibility(v3Normal, v3LightDirection, v3ViewDirection, fRoughness);
    // Reduce diffuse contribution by reflected light
    v3Diffuse *= (1.0 - v3F);
    // Combine diffuse and specular
    vec3 v3Result = v3Diffuse + (v3F * fD * fV);
    // Account for incoming light angle
    v3Result *= max(dot(v3Normal, v3LightDirection),0.0);
    // Apply light intensity
    v3Result *= v3LightIrradiance;
    return v3Result;
}


// Fragment shader
void main()
{
    // Normalize inputs
    vec3 v3Normal = normalize(v3VertexNormalIn);
    vec3 v3ViewDirection = normalize(v3CameraPosition - v3VertexPosIn);
    // Sample material textures
    vec3 v3DiffuseColour = texture(uDiffuseMap, v2VertexUVIn).rgb;
    vec3 v3SpecularColour = texture(uSpecularMap, v2VertexUVIn).rgb;

    float fRoughness = texture(uRoughnessMap, v2VertexUVIn).r;
    
    // Accumulate lighting
    vec3 v3RetColour = vec3(0.0);
    
    for (int i = 0; i < iNumPointLights; ++i)
    {
        vec3 v3LightDirection = normalize(PointLights[i].v3LightPosition - v3VertexPosIn);
        vec3 v3LightIrradiance = lightFalloff(PointLights[i].v3LightIntensity, 
                                    PointLights[i].v3Falloff, 
                                    PointLights[i].v3LightPosition, 
                                    v3VertexPosIn);
        v3RetColour += GGX(v3Normal, 
                        v3LightDirection, 
                        v3ViewDirection, 
                        v3LightIrradiance, 
                        v3DiffuseColour, 
                        v3SpecularColour, 
                        fRoughness);
    }
    // Ambient contribution
    v3RetColour += v3DiffuseColour * vec3(0.3);
    v3ColorOut = v3RetColour;
}




//
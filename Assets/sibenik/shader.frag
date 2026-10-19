#version 460 core

layout(location = 0) out vec4 FragColor;

in VS_OUT
{
    vec3 vPos;
    vec3 vNormal;
    vec2 vUV;
} fs_in;

struct Material
{
    vec3 albedo;
    float roughness;
    float metallic;
};

uniform Material uMaterial;

uniform sampler2D uAlbedoMap;
uniform bool uHasAlbedoMap;

uniform vec3 uCameraPos;

void main()
{
    vec3 albedo = uMaterial.albedo;

    if (uHasAlbedoMap)
        albedo *= texture(uAlbedoMap, fs_in.vUV).rgb;

    vec3 normal = normalize(fs_in.vNormal);

    // Temporary light direction.
    // Replace this with your actual light system later.
    vec3 lightPos = vec3(2.0, 4.0, 2.0);
    vec3 lightColor = vec3(1.0);

    vec3 L = normalize(lightPos - fs_in.vPos);
    float NdotL = max(dot(normal, L), 0.0);

    vec3 diffuse = albedo * lightColor * NdotL;

    // Small ambient term so completely unlit surfaces aren't black.
    vec3 ambient = albedo * 0.05;

    FragColor = vec4(ambient + diffuse, 1.0);
}
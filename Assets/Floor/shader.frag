#version 460 core

in vec3 vWorldPosition;
in vec3 vNormal;
in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform sampler2D uNormalMap;
uniform sampler2D uRoughnessMap;

uniform vec3 uCameraPosition;

uniform vec3 uLightPosition;
uniform vec3 uLightColor;
uniform float uLightIntensity;

const float PI = 3.14159265359;


// ------------------------------------------------------------
// Normal map
// ------------------------------------------------------------

vec3 GetNormal()
{
    vec3 tangentNormal =
        texture(uNormalMap, vTexCoord).xyz * 2.0 - 1.0;

    // Temporary tangent-space approximation.
    // For a proper normal map you should eventually construct
    // a TBN matrix using the mesh tangent/bitangent.
    return normalize(vNormal);
}


// ------------------------------------------------------------
// GGX
// ------------------------------------------------------------

float DistributionGGX(
    vec3 N,
    vec3 H,
    float roughness
)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denominator =
        NdotH2 * (a2 - 1.0) + 1.0;

    denominator = PI * denominator * denominator;

    return a2 / max(denominator, 0.0001);
}


// ------------------------------------------------------------
// Schlick-GGX
// ------------------------------------------------------------

float GeometrySchlickGGX(
    float NdotV,
    float roughness
)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return NdotV /
           (NdotV * (1.0 - k) + k);
}


float GeometrySmith(
    vec3 N,
    vec3 V,
    vec3 L,
    float roughness
)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggxV =
        GeometrySchlickGGX(NdotV, roughness);

    float ggxL =
        GeometrySchlickGGX(NdotL, roughness);

    return ggxV * ggxL;
}


// ------------------------------------------------------------
// Fresnel
// ------------------------------------------------------------

vec3 FresnelSchlick(
    float cosTheta,
    vec3 F0
)
{
    return F0 +
           (1.0 - F0) *
           pow(1.0 - cosTheta, 5.0);
}


// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

void main()
{
    // Material
    vec3 albedo =
        texture(uDiffuseMap, vTexCoord).rgb;

    float roughness =
        texture(uRoughnessMap, vTexCoord).r;

    roughness = clamp(roughness, 0.04, 1.0);

    // Floor is non-metallic.
    float metallic = 0.0;

    // Normal
    vec3 N = GetNormal();

    // View direction
    vec3 V =
        normalize(uCameraPosition - vWorldPosition);

    // Light direction
    vec3 L =
        normalize(uLightPosition - vWorldPosition);

    vec3 H =
        normalize(V + L);

    // Distance attenuation
    float distance =
        length(uLightPosition - vWorldPosition);

    float attenuation =
        1.0 / max(distance * distance, 0.001);

    vec3 radiance =
        uLightColor *
        uLightIntensity *
        attenuation;

    // --------------------------------------------------------
    // Cook-Torrance BRDF
    // --------------------------------------------------------

    vec3 F0 = vec3(0.04);

    F0 = mix(
        F0,
        albedo,
        metallic
    );

    vec3 F = FresnelSchlick(
        max(dot(H, V), 0.0),
        F0
    );

    float D = DistributionGGX(
        N,
        H,
        roughness
    );

    float G = GeometrySmith(
        N,
        V,
        L,
        roughness
    );

    vec3 numerator =
        D * G * F;

    float denominator =
        4.0 *
        max(dot(N, V), 0.0) *
        max(dot(N, L), 0.0);

    vec3 specular =
        numerator /
        max(denominator, 0.001);

    // Diffuse
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;

    vec3 diffuse =
        kD *
        albedo /
        PI;

    float NdotL =
        max(dot(N, L), 0.0);

    vec3 lighting =
        (diffuse + specular) *
        radiance *
        NdotL;

    // Small ambient term so the floor isn't completely black.
    vec3 ambient =
        albedo * 0.03;

    vec3 color =
        ambient + lighting;

    // Simple HDR -> LDR
    color =
        color /
        (color + vec3(1.0));

    color =
        pow(color, vec3(1.0 / 2.2));

    FragColor =
        vec4(color, 1.0);
}
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(std140, binding = 0) uniform CameraBlock
{
    mat4 uView;
    mat4 uProj;
};

uniform mat4 uModel;

out VS_OUT
{
    vec3 vPos;
    vec3 vNormal;
    vec2 vUV;
} vs_out;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);

    vs_out.vPos = worldPos.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    vs_out.vNormal = normalize(normalMatrix * aNormal);

    vs_out.vUV = aUV;

    gl_Position = uProj * uView * worldPos;
}
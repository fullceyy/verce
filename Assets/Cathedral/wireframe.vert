#version 460 core

layout(binding = 0) uniform TransformData
{
    mat4 m4Transform;
};

layout(binding = 1) uniform CameraData
{
    mat4 m4ViewProjection;
    vec3 v3CameraPosition;
};

layout(location = 0) in vec3 v3VertexPos;
layout(location = 1) in vec3 v3VertexNormal;
layout(location = 2) in vec2 v2VertexUV;

void main()
{
    vec4 v4WorldPosition = m4Transform * vec4(v3VertexPos, 1.0);
    gl_Position = m4ViewProjection * v4WorldPosition;
}
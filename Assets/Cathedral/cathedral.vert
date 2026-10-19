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

out vec3 v3VertexPosOut;
out vec3 v3VertexNormalOut;
out vec2 v2VertexUVout;

void main()
{
    // Transform vertex pos
    vec4 v4WorldPosition = m4Transform * vec4(v3VertexPos, 1.0f);
    gl_Position = m4ViewProjection * v4WorldPosition;
    v3VertexPosOut = v4WorldPosition.xyz;
    
    // Transform normal
    vec4 v4Normal = m4Transform * vec4(v3VertexNormal, 0.0f);
    v3VertexNormalOut = v4Normal.xyz;
    
    // UV
    v2VertexUVout = v2VertexUV;
}

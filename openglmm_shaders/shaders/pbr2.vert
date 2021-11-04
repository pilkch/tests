#version 330 core

uniform mat4 matProjection;
uniform mat4 matView;
uniform mat4 matModel;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord0;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

void main()
{
    TexCoords = texCoord0;
    WorldPos = vec3(matModel * vec4(position, 1.0));
    Normal = mat3(matModel) * normal;   

    gl_Position = matProjection * matView * vec4(WorldPos, 1.0);
}

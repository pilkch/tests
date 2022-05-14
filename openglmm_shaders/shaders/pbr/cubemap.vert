#version 330 core

uniform mat4 projection;
uniform mat4 view;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

smooth out vec3 WorldPos;
smooth out vec2 TexCoord;

void main()
{
    WorldPos = position;
    TexCoord = texCoord;
    gl_Position =  projection * view * vec4(position, 1.0);
}

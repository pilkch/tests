#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

uniform mat4 matModelViewProjection;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);
}

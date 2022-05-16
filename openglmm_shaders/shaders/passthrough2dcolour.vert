#version 330 core

uniform mat4 matModelViewProjection;

layout(location = 0) in vec4 position;

void main()
{
  gl_Position = matModelViewProjection * position;
}

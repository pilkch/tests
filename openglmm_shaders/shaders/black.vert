#version 330 core

uniform mat4 matModelViewProjection;

layout(location = 0) in vec3 position;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);
}

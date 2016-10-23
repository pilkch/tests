#version 330 core

uniform mat4 matModelViewProjection;

layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 colour;

// Vertex heat value for the fragment shader
smooth out float vertOutHeatValue;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the heat value to the fragment shader
  vertOutHeatValue = colour.r;
}

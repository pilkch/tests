#version 330 core

uniform mat4 matModelViewProjection;

layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 colour;

// Send the texture coordinates to the fragment shader
smooth out vec2 vertOutTexCoord;

// Vertex heat value for the fragment shader
smooth out float vertOutHeatValue;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  vertOutTexCoord = texCoord;

  // Pass on the heat value to the fragment shader
  vertOutHeatValue = colour.r;
}

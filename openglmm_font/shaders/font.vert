#version 330 core

uniform mat4 matModelViewProjection;

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord0;

// Texture coordinates for the fragment shader
smooth out vec4 vertOutColour;
smooth out vec2 vertOutTexCoord0;

void main()
{
  gl_Position = matModelViewProjection * position;
  vertOutColour = colour;
  vertOutTexCoord0 = texCoord0;
}

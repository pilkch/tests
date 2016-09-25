#version 330 core

uniform mat4 matModelViewProjection;

layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
}

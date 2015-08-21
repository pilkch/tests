#version 330

uniform mat4 matModelViewProjection;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord0;

// Color to fragment program
smooth out vec2 vertOutTexCoord;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord = texCoord0;
}

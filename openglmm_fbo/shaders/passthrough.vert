#version 130

uniform mat4 matModelViewProjection;

in vec4 position;
in vec2 texCoord0;

// Color to fragment program
smooth out vec2 vertOutTexCoord;

void main()
{
  gl_Position = matModelViewProjection * position;

  // Pass on the texture coordinates
  vertOutTexCoord = texCoord0;
}

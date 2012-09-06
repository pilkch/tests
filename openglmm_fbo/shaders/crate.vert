#version 130

uniform mat4 matModelViewProjection;

in vec4 position;
in vec2 texCoord0;
in vec2 texCoord1;

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;
smooth out vec2 vertOutTexCoord1;

void main()
{
  gl_Position = matModelViewProjection * position;

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
  vertOutTexCoord1 = texCoord1;
}


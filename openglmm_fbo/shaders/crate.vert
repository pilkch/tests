#version 330

uniform mat4 matModelViewProjection;

#define POSITION 0
#define TEXCOORD0 1
#define TEXCOORD1 2
layout(location = POSITION) in vec4 position;
layout(location = TEXCOORD0) in vec2 texCoord0;
layout(location = TEXCOORD1) in vec2 texCoord1;

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


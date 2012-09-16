#version 330

uniform mat4 matModelViewProjection;

#define POSITION 0
#define TEXCOORD0 1
layout(location = POSITION) in vec3 position;
layout(location = TEXCOORD0) in vec2 texCoord0;

// Color to fragment program
smooth out vec2 vertOutTexCoord;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord = texCoord0;
}

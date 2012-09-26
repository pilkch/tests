#version 330

uniform mat4 matModelViewProjection;

#define POSITION 0
//#define NORMAL 1
#define TEXCOORD0 2
#define TEXCOORD1 3
layout(location = POSITION) in vec3 position;
//layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;
layout(location = TEXCOORD1) in vec2 texCoord1;

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;
smooth out vec2 vertOutTexCoord1;

smooth out float vertOutZ; // Distance from the camera to the pixel

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
  vertOutTexCoord1 = texCoord1;

  vertOutZ = gl_Position.z;
}

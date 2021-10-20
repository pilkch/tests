#version 330 core

uniform mat4 matModelViewProjection;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec2 texCoord1;
layout(location = 4) in vec2 texCoord2;

// Texture coordinates out to fragment program
smooth out vec2 vertOutTexCoord0;
smooth out vec2 vertOutTexCoord1;
smooth out vec2 vertOutTexCoord2;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
  vertOutTexCoord1 = texCoord1;
  vertOutTexCoord2 = texCoord2;
}

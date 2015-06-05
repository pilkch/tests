#version 330

uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

// Outputs for fragment program
smooth out vec3 vertOutPosition;
smooth out vec3 vertOutNormal;
smooth out vec2 vertOutTexCoord0;

void main()
{
  vertOutPosition = (matNormal * position.xyz).xyz;

  // Calculate the normal value for this vertex, in world coordinates (multiply by the normal matrix)
  vertOutNormal = normalize(matNormal * normal);

  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
}

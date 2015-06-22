#version 330

uniform mat4 matModelView;
uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

// Outputs for fragment program
smooth out vec3 vertOutNormal;
smooth out vec2 vertOutTexCoord0;


uniform vec3 fvLightPosition;
uniform vec3 fvEyePosition;
uniform mat4 matWorldInverseTranspose;

smooth out vec3 ViewDirection;
smooth out vec3 LightDirection;
smooth out vec3 reflcoord;

void main()
{
  // Calculate the normal value for this vertex, in world coordinates (multiply by the normal matrix)
  vertOutNormal = matNormal * normal;

  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;


  vec4 fvObjectPosition = matModelView * vec4(position, 1.0);
  ViewDirection = fvEyePosition - fvObjectPosition.xyz;
  vec3 ViewDirection = normalize(ViewDirection);
  LightDirection = fvLightPosition - fvObjectPosition.xyz;
  vec3 normal2 = (vec4(vertOutNormal, 1.0) * matWorldInverseTranspose).xyz;

  vec3 fin = ViewDirection -(2.0 * (dot(normalize(normal2), ViewDirection)) * normal2);
  float p = sqrt(pow(fin.x, 2.0)+pow(fin.y, 2.0) + pow((fin.z + 1.0), 2.0));
  reflcoord = vec3(((fin.x / (2.0 * p)) + 1.0 / 2.0), ((fin.y / (2.0 * p)) + 1.0 / 2.0), ((fin.z / (2.0 * p)) + 1.0 / 2.0));
}

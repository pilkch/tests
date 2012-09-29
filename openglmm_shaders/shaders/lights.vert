#version 330

uniform mat3 matNormal;
uniform mat4 matModelViewProjection;

struct cLight
{
  vec3 position;
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
};

uniform cLight light;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;

smooth out vec3 out_normal;
smooth out vec3 out_light_half_vector;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;

  // Calculate the normal value for this vertex, in world coordinates (multiply by the normal matrix)
  out_normal = normalize(matNormal * normal);

  // Calculate the light’s half vector
  const vec3 eyeVec = vec3(0.0, 0.0, 1.0);
  out_light_half_vector = normalize(light.position + eyeVec);
}

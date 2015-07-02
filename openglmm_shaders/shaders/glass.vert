#version 330

// http://www.geeks3d.com/20101008/shader-library-chromatic-aberration-demo-glsl/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

uniform mat4 matModel;
uniform mat4 matModelViewProjection;
uniform vec3 cameraPosition;

// Outputs for fragment program
smooth out vec3 N;
smooth out vec3 E;
smooth out vec2 vertOutTexCoord0;

mat3 GetLinearPart( mat4 m )
{
  mat3 result;

  result[0][0] = m[0][0]; 
  result[0][1] = m[0][1]; 
  result[0][2] = m[0][2]; 

  result[1][0] = m[1][0]; 
  result[1][1] = m[1][1]; 
  result[1][2] = m[1][2]; 

  result[2][0] = m[2][0]; 
  result[2][1] = m[2][1]; 
  result[2][2] = m[2][2]; 

  return result;
}

void main()
{
  // output position
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Texture coordinates for glossMap
  vertOutTexCoord0 = texCoord0;

  mat3 ModelWorld3x3 = GetLinearPart(matModel);

  // find world space position
  vec4 WorldPos = matModel *  vec4(position, 1.0);

  // find world space normal
  N = ModelWorld3x3 * normal; 

  // find world space eye vector
  E = WorldPos.xyz - cameraPosition.xyz;
}

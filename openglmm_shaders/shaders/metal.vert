#version 330 core

// http://www.geeks3d.com/20101008/shader-library-chromatic-aberration-demo-glsl/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec2 texCoord1; // Unused

uniform mat4 matModel;
uniform mat4 matModelViewProjection;
uniform vec3 cameraPosition;

// Outputs for fragment program
out vec3 vertOutPosition;
smooth out vec3 vertOutNormal;
smooth out vec2 vertOutTexCoord0;
smooth out vec3 N;
smooth out vec3 E;

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
  // Output position
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Texture coordinates for glossMap
  vertOutTexCoord0 = texCoord0;

  mat3 ModelWorld3x3 = GetLinearPart(matModel);

  // World space position
  vec4 WorldPos = matModel *  vec4(position, 1.0);

  // World space normal
  N = ModelWorld3x3 * normal;

  // find world space eye vector
  E = WorldPos.xyz - cameraPosition.xyz;

  vertOutNormal = mat3(transpose(inverse(matModel))) * normal;
  vertOutPosition = vec3(matModel * vec4(position, 1.0f));
}

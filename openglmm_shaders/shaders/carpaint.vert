#version 330 core

// Stolen from:
// http://blog.2pha.com/demos/threejs/shaders/9700_car_shader/car_shader_5.html

uniform mat4 matModel;
uniform mat4 matModelViewProjection;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

// Outputs for fragment program
smooth out vec3 mvPosition;
smooth out vec2 vertOutTexCoord0;
smooth out vec3 worldNormal;
smooth out vec3 cameraToVertex;
smooth out vec3 lightToVertex;

void main()
{
  mvPosition = (matModel * vec4(position, 1.0)).xyz;
  worldNormal = mat3(matModel[0].xyz, matModel[1].xyz, matModel[2].xyz) * normal;
  //worldNormal = mat3(transpose(inverse(matModel))) * normal;
  vec3 worldPosition = (matModel * vec4(position, 1.0)).xyz;
  cameraToVertex = normalize(worldPosition - cameraPosition);
  lightToVertex = normalize(worldPosition - lightPosition);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;

  gl_Position = matModelViewProjection * vec4(position, 1.0);
}

#version 330

precision highp float;

#define POSITION 0
#define NORMAL 1
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;

uniform mat4 matView;
uniform mat4 matModelView;
uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

uniform vec3 lightPosition;

out vec3 N;
out vec3 L;

void main()
{
  N = normalize(matNormal * normal);

  vec3 positionCameraSpace = (matModelView * vec4(position, 1.0)).xyz;
  L = normalize((matView * vec4(lightPosition, 1.0)).xyz - positionCameraSpace);

  gl_Position = matModelViewProjection * vec4(position, 1.0);
}

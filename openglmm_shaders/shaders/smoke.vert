#version 330 core

precision highp float;

// Using a similar technique to Brutal Legend
// http://drewskillman.com/GDC2010_VFX.pdf

#define SMOKE_SPHERICAL_BILLBOARD

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matModelView;
uniform mat4 matProjection;

uniform mat4 matObjectRotation;

uniform vec3 lightPosition;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec3 user0; // User data for the centre of the current particle

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;

smooth out float vertOutDepth;

smooth out vec3 N;
smooth out vec3 L;

void main()
{
  // Get a copy of the model view matrix
  mat4 modelView = matModelView;

  // First column
  modelView[0][0] = 1.0;
  modelView[0][1] = 0.0;
  modelView[0][2] = 0.0;

  #ifdef FIRE_SPHERICAL_BILLBOARD
  // Second column
  modelView[1][0] = 0.0;
  modelView[1][1] = 1.0;
  modelView[1][2] = 0.0;
  #endif

  // Third column
  modelView[2][0] = 0.0;
  modelView[2][1] = 0.0;
  modelView[2][2] = 1.0;

  vec3 particleCentre = user0;

  vec4 tansformedParticleCentre = (matView * (matModel * matObjectRotation)) * vec4(particleCentre, 1.0);
  vec4 tansformedPosition = tansformedParticleCentre + vec4(particleCentre - position, 1.0);

  vec4 modelViewPosition = (modelView * vec4(0.0, 0.0, 0.0, 1.0) + tansformedPosition);

  gl_Position = matProjection * modelViewPosition;


  // Lambert lighting
  // Create a normal that points out from the center to this vertex
  vec4 modelViewParticleCentre = (modelView * vec4(0.0, 0.0, 0.0, 1.0) + tansformedParticleCentre);
  N = normalize(modelViewPosition.xyz - modelViewParticleCentre.xyz);

  vec3 positionCameraSpace = modelViewPosition.xyz;
  L = normalize((matView * vec4(lightPosition, 1.0)).xyz - positionCameraSpace);


  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
}

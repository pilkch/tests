#version 330

// Using a similar technique to Brutal Legend
// http://drewskillman.com/GDC2010_VFX.pdf

#define SMOKE_SPHERICAL_BILLBOARD

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matModelView;
uniform mat4 matProjection;

uniform mat4 matObjectRotation;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec3 user0; // User data for the centre of the current particle

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;

smooth out float vertOutDepth;

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


  // Tell the fragment shader our depth
  // http://stackoverflow.com/questions/16131963/depth-as-distance-to-camera-plane-in-glsl/16137020#16137020
  vertOutDepth = -modelViewPosition.z;


  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
}

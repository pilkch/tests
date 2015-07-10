#version 330

#define FIRE_SPHERICAL_BILLBOARD

uniform mat4 matModelView;
uniform mat4 matProjection;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;

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

  // Thrid column
  modelView[2][0] = 0.0;
  modelView[2][1] = 0.0;
  modelView[2][2] = 1.0;

  // Transform our position by the projection matrix and new model view matrix
  gl_Position = matProjection * (modelView * vec4(position, 1.0));

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
}

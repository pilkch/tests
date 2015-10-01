#version 330 core

uniform mat4 matModelViewProjection;
uniform mat4 matModel;
uniform mat4 matView;

uniform mat4 DepthBiasMVP;
uniform vec3 LightInvDirection_worldspace;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

// Texture coordinates for the fragment shader
smooth out vec2 vertOutTexCoord0;

out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec4 ShadowCoord;

void main()
{
  // Output position of the vertex, in clip space : matModelViewProjection * position
  gl_Position =  matModelViewProjection * vec4(position, 1.0);

  // Calculate our shadow coordinate
  ShadowCoord = DepthBiasMVP * vec4(position, 1.0);

  // Position of the vertex, in worldspace : matModel * position
  Position_worldspace = (matModel * vec4(position, 1.0)).xyz;

  // Vector that goes from the vertex to the camera, in camera space
  // In camera space, the camera is at the origin (0,0,0)
  EyeDirection_cameraspace = vec3(0.0, 0.0, 0.0) - (matView * matModel * vec4(position, 1.0)).xyz;

  // Vector that goes from the vertex to the light, in camera space
  LightDirection_cameraspace = (matView * vec4(LightInvDirection_worldspace, 0.0)).xyz;

  // Normal of the the vertex, in camera space
  Normal_cameraspace = (matView * matModel * vec4(normal, 0.0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;
}

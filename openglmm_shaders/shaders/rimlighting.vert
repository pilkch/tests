#version 330

precision highp float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

uniform mat4 matModel;
uniform mat4 matModelViewProjection;
                       
uniform vec3 eyePosition;

// Send vertex normal and texture coordinates to the fragment program                     
smooth out vec3 vertOutEyePosition;
smooth out vec3 vertOutNormal;
smooth out vec2 vertOutTexCoord;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);
  
  // The world matrix is the model matrix apparently
  mat4 matWorld = matModel;

  vec4 vWorldPosition = matWorld * vec4(position, 1.0);
  vertOutEyePosition = normalize(eyePosition - vec3(vWorldPosition));

  // Pass the normal
  vertOutNormal = mat3(matWorld) * normal;

  // Pass on the texture coordinates
  vertOutTexCoord = texCoord0;
}

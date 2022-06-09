#version 330 core

uniform mat4 matModelViewProjection;
uniform mat4 matModel;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textureCoord0;

// Outputs for fragment program
out vec3 vertOutNormal;
smooth out vec2 vertOutTexCoord0;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates, normal, and texture coordinates
  vertOutNormal = mat3(transpose(inverse(matModel))) * normal;
  vertOutTexCoord0 = textureCoord0;
}

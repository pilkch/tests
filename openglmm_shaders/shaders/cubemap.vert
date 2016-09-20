#version 330 core

uniform mat4 matModelViewProjection;
uniform mat4 matModel;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

// Outputs for fragment program
smooth out vec2 vertOutTexCoord0;
out vec3 Normal;
out vec3 Position;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;

  Normal = mat3(transpose(inverse(matModel))) * normal;
  Position = vec3(matModel * vec4(position, 1.0f));
}

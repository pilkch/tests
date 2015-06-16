#version 330

uniform mat4 matModelViewProjection;
uniform mat4 matModel;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

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

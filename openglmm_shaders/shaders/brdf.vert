#version 330 core

// Stolen from:
// https://github.com/McNopper/OpenGL/blob/master/Example32/shader/brdf.vert.glsl

uniform mat4 matViewProjection;
uniform mat4 matModel;
uniform mat3 matNormal;

uniform vec3 cameraPos;


#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;


out vec3 v_eye;

out vec3 v_tangent;
out vec3 v_bitangent;
out vec3 v_normal;

void calculateBasis(out vec3 tangent, out vec3 bitangent, in vec3 normal)
{
  bitangent = vec3(0.0, 1.0, 0.0);

  float normalDotUp = dot(normal, bitangent);

  if (normalDotUp == 1.0) {
    bitangent = vec3(0.0, 0.0, -1.0);
  } else if (normalDotUp == -1.0) {
    bitangent = vec3(0.0, 0.0, 1.0);
  }

  tangent = cross(bitangent, normal);  
  bitangent = cross(normal, tangent);
} 

void main(void)
{
  v_normal = matNormal * normal;

  calculateBasis(v_tangent, v_bitangent, v_normal);

  vec4 vertex = matModel * vec4(position, 1.0);

  v_eye = (vec4(cameraPos, 1.0) - vertex).xyz; 

  gl_Position = matViewProjection * vertex;
}

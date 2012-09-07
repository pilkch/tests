#version 330

// http://www.swiftless.com/tutorials/glsl/6_materials.html

uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

uniform vec3 lightPosition;

#define POSITION 0
#define NORMAL 1
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;

out vec3 out_normal;
out vec3 out_light_half_vector;

void main()
{
  // Set the position of the current vertex
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Calculate the normal value for this vertex, in world coordinates (multiply by the normal matrix)
  out_normal = normalize(matNormal * normal);

  // Calculate the light’s half vector
  const vec3 eyeVec = vec3(0.0, 0.0, 1.0);
  out_light_half_vector = normalize(lightPosition + eyeVec);
}

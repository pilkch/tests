
// http://www.swiftless.com/tutorials/glsl/6_materials.html

uniform vec3 lightPosition;

varying vec3 vertex_light_half_vector;
varying vec3 vertex_normal;

void main()
{
  // Calculate the normal value for this vertex, in world coordinates (multiply by gl_NormalMatrix)
  vertex_normal = normalize(gl_NormalMatrix * gl_Normal);

  // Calculate the light’s half vector
  vec3 eyeVec = float3(0.0, 0.0, 1.0);
  vertex_light_half_vector = normalize(lightPosition + eyeVec);

  // Set the position of the current vertex
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  // Set the front color to the color passed through with glColor*f
  gl_FrontColor = gl_Color;
}

#version 330

// http://www.swiftless.com/tutorials/glsl/6_materials.html

struct cLight
{
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
  vec3 position;
  vec4 halfVector;
  vec3 spotDirection;
  float spotExponent;
  float spotCutoff;
  float spotCosCutoff;
  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;
};

uniform cLight light;

struct cMaterial
{
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
  float fShininess;
};

uniform cMaterial material;

in vec3 out_light_half_vector;
in vec3 out_normal;

out vec4 out_color0;

void main()
{
  // Calculate the ambient term
  vec4 ambient_colour = (material.ambientColour * light.ambientColour);

  // Calculate the diffuse term
  vec4 diffuse_colour = material.diffuseColour * light.diffuseColour;

  // Calculate the specular value
  vec4 specular_colour = material.specularColour * light.specularColour * pow(max(dot(out_normal, out_light_half_vector), 0.0) , material.fShininess);

    // Set the diffuse value (darkness). This is done with a dot product between the normal and the light
  // and the maths behind it is explained in the maths section of the site.
  float diffuse_value = max(dot(out_normal, light.position), 0.0);

  // Set the output colour of our current pixel
  out_color0 = ambient_colour + (diffuse_colour * diffuse_value) + specular_colour;
}

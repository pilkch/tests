#version 330

// http://www.swiftless.com/tutorials/glsl/6_materials.html

uniform vec4 ambientColour;

uniform vec3 lightPosition;
uniform vec4 lightAmbientColour;
uniform vec4 lightDiffuseColour;
uniform vec4 lightSpecularColour;

uniform vec4 materialAmbientColour;
uniform vec4 materialDiffuseColour;
uniform vec4 materialSpecularColour;
uniform float fMaterialShininess;

in vec3 out_light_half_vector;
in vec3 out_normal;

out vec4 out_color0;

void main()
{
  // Calculate the ambient term
  vec4 ambient_colour = (materialAmbientColour * lightAmbientColour) + (ambientColour * materialAmbientColour);

  // Calculate the diffuse term
  vec4 diffuse_colour = materialDiffuseColour * lightDiffuseColour;

  // Calculate the specular value
  vec4 specular_colour = materialSpecularColour * lightSpecularColour * pow(max(dot(out_normal, out_light_half_vector), 0.0) , fMaterialShininess);

    // Set the diffuse value (darkness). This is done with a dot product between the normal and the light
  // and the maths behind it is explained in the maths section of the site.
  float diffuse_value = max(dot(out_normal, lightPosition), 0.0);

  // Set the output colour of our current pixel
  out_color0 = ambient_colour + (diffuse_colour * diffuse_value) + specular_colour;
}

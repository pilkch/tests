#version 330

// Using a similar technique to Brutal Legend
// http://drewskillman.com/GDC2010_VFX.pdf

uniform sampler2D texUnit0; // Diffuse texture

uniform vec3 ambientColour;
uniform vec3 skyColour; // Add some light towards the top of the particle

struct cSun
{
  vec3 position;
  vec3 colour;
};

uniform cSun sun;

smooth in vec2 vertOutTexCoord0;

//const float tolerance = 0.01;

out vec4 fragmentColour;

void main()
{
  vec4 albedo = texture(texUnit0, vertOutTexCoord0);
  //if (albedo.a < tolerance) discard;

  fragmentColour = albedo;
}

#version 330

precision highp float;

#include <lighting.header>

// Using a similar technique to Brutal Legend
// http://drewskillman.com/GDC2010_VFX.pdf

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2DRect texUnit1; // Depth buffer

uniform vec3 ambientColour;
uniform vec3 skyColour; // Add some light towards the top of the particle

uniform vec3 lightColour;
uniform vec3 colour;

struct cSun
{
  vec3 position;
  vec3 colour;
};

uniform cSun sun;

smooth in vec2 vertOutTexCoord0;

//const float tolerance = 0.01;
smooth in vec3 N;
smooth in vec3 L;

out vec4 fragmentColour;

void main()
{
  vec4 albedo = texture(texUnit0, vertOutTexCoord0);
  //if (albedo.a < tolerance) discard;

  // Lambert lighting
  vec3 ambient = ambientColour;
  float fLambertDiffuse = CalculateLambertDiffuse(normalize(L), normalize(N));
  vec3 diffuse = lightColour * fLambertDiffuse;


  //fragmentColour = vec4((ambient + diffuse) * colour * albedo.rgb, albedo.a * fSoftness);
  //fragmentColour = vec4((ambient + diffuse) * colour * albedo.rgb, 1.0 + 0.0001 * (albedo.a + fSoftness));
  fragmentColour = vec4(vec3(fDiscard, 1.0 - fDiscard, 0.0) + 0.001 * ((ambient + diffuse) * colour * albedo.rgb), albedo.a + fSoftness);
}

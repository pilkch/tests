#version 330

#include <lighting.header>

precision highp float;

uniform vec3 diffuseColour;
uniform vec3 ambientColour;
uniform vec3 lightColour;

in vec3 N;
in vec3 L;

out vec4 fragmentColour;

void main()
{
  float lambert = CalculateLambertDiffuse(normalize(L), normalize(N));
  fragmentColour = vec4((0.7 * diffuseColour) + (0.5 * mix(ambientColour, lightColour, lambert)), 1.0);
}

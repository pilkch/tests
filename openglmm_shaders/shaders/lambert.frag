#version 330

#include <lighting.header>

precision highp float;

in vec3 N;
in vec3 L;

out vec4 fragmentColour;

const vec3 diffuseColour = vec3(0.8f, 0.1f, 0.2f);

void main()
{
  float lambert = CalculateLambertDiffuse(normalize(L), normalize(N));
  fragmentColour = vec4(diffuseColour * lambert, 1.0);
}

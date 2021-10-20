#version 330 core

#include <lighting.header>

precision highp float;

uniform vec3 colour;
uniform vec3 ambientColour;
uniform vec3 lightColour;

in vec3 N;
in vec3 L;

out vec4 fragmentColour;

void main()
{
  vec3 ambient = ambientColour;
  float fLambertDiffuse = CalculateLambertDiffuse(normalize(L), normalize(N));
  vec3 diffuse = lightColour * fLambertDiffuse;
  fragmentColour = vec4((ambient + diffuse) * colour, 1.0);
}

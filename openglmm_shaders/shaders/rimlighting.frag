#version 330 core

#include <lighting.header>

precision highp float;

uniform sampler2D texUnit0; // Diffuse texture

uniform vec3 ambientColour;
uniform vec3 rimColour;

smooth in vec3 vertOutEyePosition;
smooth in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 colour = texture(texUnit0, vertOutTexCoord).rgb;

  vec3 ambient = ambientColour;
  vec3 diffuse = colour;

  // Rim lighting
  float fRimLighting = CalculateRimLighting(vertOutEyePosition, vertOutNormal);
  vec3 rim = fRimLighting * rimColour;

  fragmentColour = vec4((ambient + diffuse + rim) * colour, 1.0);
}

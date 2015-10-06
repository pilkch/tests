#version 330

precision highp float;

#include <math.header>

uniform sampler2D texUnit0; // Diffuse texture

uniform vec4 lightColour;

smooth in vec2 vertOutTexCoord0;

const float fTolerance = 0.6;

out vec4 fragmentColour;

void main()
{
  vec4 albedo = texture(texUnit0, vertOutTexCoord0);
  if (albedo.a < fTolerance) discard;

  // TODO: There are still some bugs where particles are not culled if they are about 5 meters from the camera and behind an opaque object
  // It may be due to one of these:
  // 1) Non-linear depth buffer coordinates
  // 2) Incorrectly checking a linear depth against a non-linear depth, so it kind of works

  fragmentColour = vec4(lightColour.rgb * albedo.rgb, albedo.a);
}

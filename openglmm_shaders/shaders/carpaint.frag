#version 330

#include <math.header>

uniform sampler2D texUnit0; // Diffuse texture
uniform samplerCube texUnit1; // Cubemap texture

uniform vec3 cameraPosition;

smooth in vec3 vertOutPosition;
smooth in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColour;

void main()
{
  vec3 diffuseColour = (texture(texUnit0, vertOutTexCoord0).rgb) * 0.5;

  // Mix between the diffuse colour and the cubemap based on noise that is a function of x and y
  //float fMix = (simplexNoise(vertOutTexCoord0) + 1.0) * 0.25; // Biased towards the diffuse colour
  //const float fMix = 0.99; // For testing
  const float fMix = 0.2;
  fragmentColour = vec4(mix(diffuseColour, texture(texUnit1, reflect(vertOutPosition - cameraPosition, normalize(vertOutNormal))).rgb, fMix), 1.0);
}

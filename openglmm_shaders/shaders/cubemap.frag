#version 330

#include <math.header>

uniform samplerCube texUnit0; // Cubemap texture

uniform vec3 cameraPosition;

smooth in vec3 vertOutPosition;
smooth in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColour;

void main()
{
  fragmentColour = vec4(texture(texUnit0, reflect(vertOutPosition - cameraPosition, normalize(vertOutNormal))).rgb, 1.0);
}

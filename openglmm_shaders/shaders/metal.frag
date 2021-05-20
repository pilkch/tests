#version 330 core

// http://www.geeks3d.com/20101008/shader-library-chromatic-aberration-demo-glsl/

#include <math.header>

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Specular texture
uniform samplerCube texUnit2; // Cubemap texture

uniform vec3 cameraPosition;

in vec3 vertOutPosition;
smooth in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;
smooth in vec3 N;
smooth in vec3 E;

out vec4 fragmentColour;

vec3 CalculateCubeMap(vec3 normal)
{
  vec3 I = normalize(vertOutPosition - cameraPosition);
  vec3 R = reflect(I, normalize(normal));
  return texture(texUnit2, R).rgb;
}

void main()
{
  // Use simplex noise to jitter the normal
  // NOTE: A texture would be simpler and more efficient
  vec3 jitter = vec3(
    0.1 * (-0.5 + simplexNoise(40.0 * vertOutTexCoord0)),
    0.1 * (-0.5 + simplexNoise(40.0 * (vec2(100.0, 0.0) + vertOutTexCoord0))),
    0.1 * (-0.5 + simplexNoise(40.0 * (vec2(200.0, 0.0) + vertOutTexCoord0)))
  );

  // Normalize incoming vectors
  vec3 normal = normalize(N + jitter);
  vec3 incident = normalize(E);

  vec3 base_color = texture(texUnit0, vertOutTexCoord0).rgb;

  // Do a gloss map look up and compute the reflectivity
  vec3 gloss_color = texture(texUnit1, vertOutTexCoord0).rgb;
  float reflectivity = (gloss_color.r + gloss_color.g + gloss_color.b) / 3.0;

  vec3 cubemapColour = CalculateCubeMap(vertOutNormal + jitter);

  // Write the final pixel
  fragmentColour = vec4( mix(base_color, cubemapColour, reflectivity), 1.0);
}

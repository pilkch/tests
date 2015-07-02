#version 330

// http://www.geeks3d.com/20101008/shader-library-chromatic-aberration-demo-glsl/

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Specular texture
uniform samplerCube texUnit2; // Cubemap texture

uniform vec3 cameraPosition;
uniform vec3 fresnelValues;

in vec3 vertOutPosition;
smooth in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;
smooth in vec3 N;
smooth in vec3 E;

out vec4 fragmentColour;

// fresnel approximation
// F(a) = F(0) + (1- cos(a))^5 * (1- F(0))
// Calculate fresnel term. You can approximate it with 1.0-dot(normal, viewpos)
//
float fast_fresnel(vec3 I, vec3 N)
{
  float bias = fresnelValues.x;
  float power = fresnelValues.y;
  float scale = 1.0 - bias;

  return bias + pow(1.0 - dot(I, N), power) * scale;
}

// Calculate fresnel term. You can approximate it with 1.0-dot(normal, viewpos)
//
float very_fast_fresnel(vec3 I, vec3 N)
{
  return 1.0 - dot(N, I);
}

vec3 CalculateCubeMap()
{
  vec3 I = normalize(vertOutPosition - cameraPosition);
  vec3 R = reflect(I, normalize(vertOutNormal));
  return texture(texUnit2, R).rgb;
}

void main()
{
  // Normalize incoming vectors
  vec3 normal = normalize(N);
  vec3 incident = normalize(E);

  // Find the reflection
  vec3 reflVec = normalize(reflect(incident, normal));
  vec3 reflectColor = texture(texUnit2, reflVec).xyz;

  vec3 base_color = texture(texUnit0, vertOutTexCoord0).rgb;

  // Do a gloss map look up and compute the reflectivity
  vec3 gloss_color = texture(texUnit1, vertOutTexCoord0).rgb;
  float reflectivity = (gloss_color.r + gloss_color.g + gloss_color.b) / 3.0;

  // Find the Fresnel term
  float fresnelTerm = fast_fresnel(-incident, normal);
  //float fresnelTerm = very_fast_fresnel(-incident, normal);

  vec3 cubemapColour = CalculateCubeMap();

  // Write the final pixel
  vec3 color = mix(cubemapColour, reflectColor, fresnelTerm);
  fragmentColour = vec4( mix(base_color, color, reflectivity), 1.0);
}

#version 330 core

// http://www.geeks3d.com/20101008/shader-library-chromatic-aberration-demo-glsl/

#include <math.header>

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Specular texture
uniform samplerCube texUnit2; // Cubemap texture


uniform vec3 fresnelValues;
uniform vec3 IoR_Values;

smooth in vec2 vertOutTexCoord0;
smooth in vec3 N;
smooth in vec3 E;

out vec4 fragmentColour;

vec3 refract(vec3 i, vec3 n, float eta)
{
  float cosi = dot(-i, n);
  float cost2 = 1.0 - eta * eta * (1.0 - cosi*cosi);
  vec3 t = eta*i + ((eta*cosi - sqrt(abs(cost2))) * n);
  return t * vec3(cost2 > 0.0);
}

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

//
//
// Calculate fresnel term. You can approximate it with 1.0-dot(normal, viewpos)
//
float very_fast_fresnel(vec3 I, vec3 N)
{
  return 1.0 - dot(N, I);
}

void main()
{
  //------ normalize incoming vectors
  //
  vec3 normal = normalize(N);
  vec3 incident = normalize(E);

  //------ Find the reflection
  //
  vec3 reflVec = normalize(reflect(incident, normal));
  vec3 reflectColor = texture(texUnit2, reflVec).xyz;

  //------ Find the refraction
  //
  vec3 refractColor;
  refractColor.x = texture(texUnit2, refract(incident, normal, IoR_Values.x)).x;
  refractColor.y = texture(texUnit2, refract(incident, normal, IoR_Values.y)).y;
  refractColor.z = texture(texUnit2, refract(incident, normal, IoR_Values.z)).z;

  vec3 base_color = texture(texUnit0, vertOutTexCoord0.st).rgb;

  //------ Do a gloss map look up and compute the reflectivity.
  //
  vec3 gloss_color = texture(texUnit1, vertOutTexCoord0.st * 2.0).rgb;
  float reflectivity = (gloss_color.r + gloss_color.g + gloss_color.b)/3.0;

  //------ Find the Fresnel term
  //
  float fresnelTerm = fast_fresnel(-incident, normal);
  //float fresnelTerm = very_fast_fresnel(-incident, normal);

  //------ Write the final pixel.
  //
  vec3 color = mix(refractColor, reflectColor, fresnelTerm);
  fragmentColour = vec4( mix(base_color, color, reflectivity), 1.0);
}

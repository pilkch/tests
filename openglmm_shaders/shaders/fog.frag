#version 330

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Detail texture

struct cFog
{
  vec4 colour;
  float fStart;
  float fEnd;
  float fDensity;
};

uniform cFog fog;

smooth in vec2 vertOutTexCoord0;
smooth in vec2 vertOutTexCoord1;

smooth in float vertOutZ; // Distance from the camera to the pixel

out vec4 fragmentColour;


float CalculateFogLinear(float distanceToEye)
{
  float f = (fog.fEnd - distanceToEye) / (fog.fEnd - fog.fStart);
  return clamp(f, 0.0, 1.0);
}

float CalculateFogExp(float distanceToEye)
{
  float f = exp(-(distanceToEye * fog.fDensity));
  return clamp(f, 0.0, 1.0);
}

float CalculateFogExp2(float distanceToEye)
{
  float f = distanceToEye * fog.fDensity;
  f = exp(-(f * f));
  return clamp(f, 0.0, 1.0);
}


void main()
{
  vec3 diffuse = texture2D(texUnit0, vertOutTexCoord0).rgb;
  vec3 detail = texture2D(texUnit1, vertOutTexCoord1).rgb;

  fragmentColour = vec4(mix(fog.colour.rgb, diffuse * detail, CalculateFogLinear(vertOutZ)), 1.0);
}

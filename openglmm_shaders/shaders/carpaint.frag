#version 330

#include <math.header>

uniform sampler2D texUnit0; // Diffuse texture
uniform samplerCube texUnit1; // Cubemap texture

smooth in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;


vec4 fvLowTone = vec4(1.0, 0.0, 1.0, 1.0);
vec4 fvSpecular = vec4(1.0, 1.0, 0.0, 1.0);
vec4 fvHighTone = vec4(0.5, 0.5, 1.0, 1.0);
float fSpecularPower = 2.0;

smooth in vec3 ViewDirection;
smooth in vec3 LightDirection;
smooth in vec3 reflcoord;

out vec4 fragmentColour;

void main()
{
  vec4 diffuseColour = vec4(texture(texUnit0, vertOutTexCoord0).rgb, 1.0);

  vec3 fvLightDirection = normalize( LightDirection );
  vec3 fvNormal = normalize( vertOutNormal );
  float fNDotL = dot( fvNormal, fvLightDirection );
  vec3 fvReflection = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection);
  vec3 fvViewDirection = normalize( ViewDirection );
  float fRDotV = max( 0.0, dot( fvReflection, fvViewDirection ) );
  vec4 fvTotalAmbient = fvLowTone * diffuseColour;
  vec4 fvTotalDiffuse = fvHighTone * (fNDotL) * diffuseColour;
  vec4 fvTotalSpecular = fvSpecular * ( pow( fRDotV, fSpecularPower ) );
  float fresnel =(1/dot( vertOutNormal, fvViewDirection) )/5;

  //original: fragmentColour = (fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular) + (fresnel * texture(texUnit1, reflcoord)) + (fresnel / 2);
  fragmentColour = 2.0 * (fvTotalAmbient + fvTotalDiffuse + 0.1 * fvTotalSpecular) + 0.1 * (fresnel * texture(texUnit1, reflcoord)) + 0.1 * (fresnel / 2);
}

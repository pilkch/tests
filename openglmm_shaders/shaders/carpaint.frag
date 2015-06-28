#version 330

vec4 fvLowTone = vec4(1.0, 0.5, 1.0, 1.0);
vec4 fvSpecular = vec4(0.5, 0.5, 1.0, 1.0);
vec4 fvHighTone = vec4(1.0, 0.0, 1.0, 1.0);
float fSpecularPower = 10.0;

uniform sampler2D texUnit0; // Diffuse texture
uniform samplerCube texUnit1; // Cubemap texture

uniform vec3 cameraPos;

in vec3 vertOutPosition;
in vec2 vertOutTexcoord;
in vec3 ViewDirection;
in vec3 LightDirection;
in vec3 vertOutNormal;
//in vec3 reflcoord;

out vec4 fragmentColour;

vec4 CalculateCubeMap()
{
  vec3 I = normalize(vertOutPosition - cameraPos);
  vec3 R = reflect(I, normalize(vertOutNormal));
  return texture(texUnit1, R);
}

void main()
{
  vec3  fvLightDirection = normalize( LightDirection );
  vec3  fvNormal         = normalize(vertOutNormal);
  float fNDotL           = dot( fvNormal, fvLightDirection );

  vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection );
  vec3  fvViewDirection  = normalize( ViewDirection );
  float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );

  vec4  fvBaseColor      = texture(texUnit0, vertOutTexcoord);

  vec4  fvTotalAmbient   = fvLowTone * fvBaseColor;
  vec4  fvTotalDiffuse   = fvHighTone * (fNDotL) * fvBaseColor;
  vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );

  // ORIGINAL: float fresnel = (1/dot(vertOutNormal, fvViewDirection) )/5;
  float fresnel = 0.1 * (1/dot(vertOutNormal, fvViewDirection) )/5;
  
  //ORIGINAL: fragmentColour = ( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular ) + (fresnel * texture(texUnit1, reflcoord)) + (fresnel / 2);
  fragmentColour = (fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular) + (fresnel * CalculateCubeMap()) + (fresnel / 2);
}

#version 330

vec4 fvLowTone = vec4(1.0, 0.0, 1.0, 1.0);
vec4 fvSpecular = vec4(1.0, 1.0, 0.0, 1.0);
vec4 fvHighTone = vec4(0.5, 0.5, 1.0, 1.0);
float fSpecularPower = 2.0;

uniform sampler2D texUnit0; // Diffuse texture
uniform samplerCube texUnit1; // Cubemap texture

in vec2 Texcoord;
in vec3 ViewDirection;
in vec3 LightDirection;
in vec3 Normal;
in vec3 reflcoord;

out vec4 fragmentColour;

void main()
{
  vec3  fvLightDirection = normalize( LightDirection );
  vec3  fvNormal         = normalize( Normal );
  float fNDotL           = dot( fvNormal, fvLightDirection );

  vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection );
  vec3  fvViewDirection  = normalize( ViewDirection );
  float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );

  vec4  fvBaseColor      = texture(texUnit0, Texcoord);

  vec4  fvTotalAmbient   = fvLowTone * fvBaseColor;
  vec4  fvTotalDiffuse   = fvHighTone * (fNDotL) * fvBaseColor;
  vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );

  // ORIGINAL: float fresnel = (1/dot( Normal, fvViewDirection) )/5;
  float fresnel = 0.01 * (1/dot( Normal, fvViewDirection) )/5;

  // ORIGINAL: fragmentColour = ( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular )+(fresnel * texture(texUnit1, reflcoord))+(fresnel /2);
  fragmentColour = ( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular )+(fresnel * texture(texUnit1, reflcoord))+(fresnel /2);
}

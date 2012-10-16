#version 130

precision highp float;

uniform sampler2D texUnit0; // Diffuse texture

struct cLightDirectional
{
  int bOn; // 1 if on, 0 if off

  vec3 direction;
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
};
uniform cLightDirectional lightDirectional;

struct cLightPointLight
{
  int bOn; // 1 if on, 0 if off

  vec3 position;
  vec4 colour;

  float fAmbient;

  float fConstantAttenuation;
  float fLinearAttenuation;
  float fExpAttenuation;
};
uniform cLightPointLight lightPointLight;

struct cLightSpotLight
{
  int bOn; // 1 if on, 0 if off

  vec3 position;
  vec3 direction;
  vec4 colour;

  float fAmbient;

  float fConstantAttenuation;
  float fLinearAttenuation;
  float fExpAttenuation;

  float fConeCosineAngle;
};
uniform cLightSpotLight lightSpotLight;

struct cMaterial
{
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
  float fShininess;
};
uniform cMaterial material;

smooth in vec3 vertOutPosition;
smooth in vec2 vertOutTexCoord0;
smooth in vec3 vertOutNormal;
smooth in vec3 vertOutPointLightDirection;
smooth in float vertOutPointLightDistance;
smooth in vec3 vertOutSpotLightDirection;
smooth in vec3 vertOutSpotLightDirectionFromVertex;
smooth in float vertOutSpotLightDistance;

out vec4 fragmentColour;


vec4 ApplyLightDirectionalLight()
{
  // If light isn't turned on, return no color
  if (lightDirectional.bOn == 0) return vec4(0.0, 0.0, 0.0, 0.0);

  vec3 L = normalize(lightDirectional.direction);
  vec3 N = normalize(vertOutNormal);
  vec3 V = normalize(-vertOutPosition);
  vec3 R = normalize(-reflect(L, N));

  float nDotL = max(0.0, dot(N, L));
  float rDotV = max(0.0, dot(R, V));

  vec4 ambient = lightDirectional.ambientColour * material.ambientColour;
  vec4 diffuse = lightDirectional.diffuseColour * material.diffuseColour * nDotL;
  vec4 specular = lightDirectional.specularColour * material.specularColour * pow(rDotV, material.fShininess);

  return (ambient + diffuse + specular);
}

vec4 ApplyLightPointLight()
{
  // If light isn't turned on, return no color
  if (lightPointLight.bOn == 0) return vec4(0.0, 0.0, 0.0, 0.0);

  float fDist = vertOutPointLightDistance;
  vec3 L = normalize(vertOutPointLightDirection);
  vec3 N = normalize(vertOutNormal);

  float nDotL = max(0.0, dot(N, L));

  float fAttTotal = lightPointLight.fConstantAttenuation + lightPointLight.fLinearAttenuation * fDist + lightPointLight.fExpAttenuation * fDist * fDist;

  return vec4(lightPointLight.colour.rgb, 1.0) * (lightPointLight.fAmbient + nDotL) / fAttTotal;
}

vec4 ApplyLightSpotLight()
{
  // If light isn't turned on, return no color
  if (lightSpotLight.bOn == 0) return vec4(0.0, 0.0, 0.0, 0.0);

  // Distance from fragment's position
  float fDistance = 0.001 * vertOutSpotLightDistance;

  // Get direction vector to fragment
  vec3 vDir = normalize(vertOutSpotLightDirection);

  // Cosine between spotlight direction and directional vector to fragment
  float fCosine = dot(normalize(vertOutSpotLightDirectionFromVertex), vDir);

  // Difference between max cosine and current cosine
  float fDif = 1.0 - lightSpotLight.fConeCosineAngle;

  // This is how strong light is depending whether its nearer to the center of
  // cone or nearer to its borders (onway factor in article), clamp to 0.0 and 1.0
  float fFactor = clamp((fCosine - lightSpotLight.fConeCosineAngle) / fDif, 0.0, 1.0);

  // If we're inside the cone, calculate color
  if (fCosine + 100.0 > lightSpotLight.fConeCosineAngle) return vec4(lightSpotLight.colour.rgb, 1.0) * fFactor / (fDistance * lightSpotLight.fLinearAttenuation);

  // No color otherwise
  return vec4(0.0, 0.0, 0.0, 0.0);

  /*float fDist = vertOutSpotLightDistance;
  vec3 L = normalize(vertOutSpotLightDirection);
  vec3 N = normalize(vertOutNormal);

  float nDotL = max(0.0, dot(N, L));

  float fAngle = max(0.0, dot(L, normalize(vertOutSpotLightDirectionFromVertex)));

  float fAttTotal = lightSpotLight.fConstantAttenuation + lightSpotLight.fLinearAttenuation * fDist + lightSpotLight.fExpAttenuation * fDist * fDist;

  vec4 colour = 0.001 * vec4(lightSpotLight.colour.rgb, 1.0) * (lightSpotLight.fAmbient + nDotL) / fAttTotal;

  // Test whether vertex is located in the cone
  if (acos(fAngle) <= radians(lightSpotLight.fCutOffDegrees)) colour += vec4(1.0, 1.0, 0.0, 1.0); // lit (yellow)

  return colour;*/
}


void main()
{
  vec3 texel = texture2D(texUnit0, vertOutTexCoord0).rgb;

  fragmentColour = vec4(texel, 1.0) + ApplyLightDirectionalLight() + ApplyLightPointLight() + ApplyLightSpotLight();
}

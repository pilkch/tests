#version 130

precision highp float;

uniform sampler2D texUnit0; // Diffuse texture

struct cLight
{
  vec3 direction;
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
};
uniform cLight light;

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
smooth in vec3 vertOutLightDirection;

out vec4 fragmentColour;


vec4 ApplyLightDirectionalLight()
{
  vec3 L = normalize(vertOutLightDirection);
  vec3 N = normalize(vertOutNormal);
  vec3 V = normalize(-vertOutPosition);
  vec3 R = normalize(-reflect(L, N));

  float nDotL = max(0.0, dot(N, L));
  float rDotV = max(0.0, dot(R, V));

  vec4 ambient = light.ambientColour * material.ambientColour;
  vec4 diffuse = light.diffuseColour * material.diffuseColour * nDotL;
  vec4 specular = light.specularColour * material.specularColour * pow(rDotV, material.fShininess);

  return (ambient + diffuse + specular);
}

vec4 ApplyLightPointLight()
{
  return vec4(1.0, 1.0, 1.0, 1.0);
}

vec4 ApplyLightSpotLight()
{
  return vec4(1.0, 1.0, 1.0, 1.0);
}


void main()
{
  vec3 texel = texture2D(texUnit0, vertOutTexCoord0).rgb;

  fragmentColour = vec4(texel, 1.0) * ApplyLightDirectionalLight();
}

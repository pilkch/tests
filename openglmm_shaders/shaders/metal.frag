#version 130

precision highp float;

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


void main()
{
  fragmentColour = ApplyLightDirectionalLight();
}

#version 330

precision highp float;

uniform mat4 matView;
uniform mat4 matModel;
uniform mat4 matModelView;
uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

struct cLightDirectional
{
  vec3 direction;
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
};
uniform cLightDirectional lightDirectional;

struct cLightPointLight
{
  vec3 position;
  vec4 colour;

  float fAmbient;

  float fConstantAttenuation;
  float fLinearAttenuation;
  float fExpAttenuation;
};
uniform cLightPointLight lightPointLight;

struct cMaterial
{
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
  float fShininess;
};
uniform cMaterial material;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

smooth out vec3 vertOutPosition;
smooth out vec2 vertOutTexCoord0;
smooth out vec3 vertOutNormal;
smooth out vec3 vertOutPointLightDirection;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Transform the vertex position into eye space. We use this later on to
  // calculate the view (eye) vector.
  vec4 pos = matModelView * vec4(position, 1.0);
  vertOutPosition = pos.xyz / pos.w;

  vec3 pointLightPosition = (matView * vec4(lightPointLight.position, 1.0)).xyz;
  vec3 vertOutWorldPosition = (matModelView * vec4(position, 1.0)).xyz;

  vertOutPointLightDirection = pointLightPosition - vertOutWorldPosition;

  vertOutNormal = matNormal * normal;
  vertOutTexCoord0 = texCoord0;
}

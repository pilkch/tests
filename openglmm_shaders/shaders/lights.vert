#version 330 core

precision highp float;

uniform mat4 matView;
uniform mat4 matModelView;
uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

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
smooth out float vertOutPointLightDistance;
smooth out vec3 vertOutSpotLightDirection;
smooth out vec3 vertOutSpotLightDirectionFromVertex;
smooth out float vertOutSpotLightDistance;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Transform the vertex position into eye space. We use this later on to
  // calculate the view (eye) vector.
  vec4 pos = matModelView * vec4(position, 1.0);
  vertOutPosition = pos.xyz / pos.w;

  vec3 vertOutWorldPosition = (matModelView * vec4(position, 1.0)).xyz;

  vec3 pointLightPosition = (matView * vec4(lightPointLight.position, 1.0)).xyz;
  vertOutPointLightDirection = pointLightPosition - vertOutWorldPosition;

  vertOutPointLightDistance = length(vertOutPointLightDirection);

  // Direction that the spot light is pointing
  vertOutSpotLightDirection = lightSpotLight.direction;

  // Direction from our vertex to the spot light
  vec3 spotLightDirectionFromVertex = lightSpotLight.position - vertOutWorldPosition;
  vertOutSpotLightDirectionFromVertex = spotLightDirectionFromVertex;

  vertOutSpotLightDistance = length(spotLightDirectionFromVertex);

  vertOutNormal = matNormal * normal;
  vertOutTexCoord0 = texCoord0;
}

#version 330

precision highp float;

uniform mat4 matView;
uniform mat4 matModelView;
uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

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

#define POSITION 0
#define NORMAL 1
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;

smooth out vec3 vertOutPosition;
smooth out vec3 vertOutNormal;
smooth out vec3 vertOutLightDirection;

void main()
{
  // All vertex shaders should write the transformed homogeneous clip space
  // vertex position into the gl_Position variables.
  vec4 pos = vec4(position.x, position.y, position.z, 1.0);

  gl_Position = matModelViewProjection * pos;

  // Transform the vertex position into eye space. We use this later on to
  // calculate the view (eye) vector.
  pos = matModelView * pos;
  vertOutPosition = pos.xyz / pos.w;

  // Transform the light direction into eye space. Directional lights are
  // specified in world space. For example, a directional light aimed along
  // the world negative z axis has the direction vector (0, 0, -1).
  vertOutLightDirection = vec3(matView * vec4(-light.direction, 0.0f));

  vertOutNormal = matNormal * normal;
}

#version 330

uniform mat4 matModel;
uniform mat4 matModelView;
uniform mat4 matModelViewProjection;
//uniform mat4 matWorldInverseTranspose;
uniform mat3 matNormal;

uniform vec3 fvLightPosition;
uniform vec3 fvEyePosition;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

out vec3 vertOutPosition;
out vec2 vertOutTexCoord0;
out vec3 ViewDirection;
out vec3 LightDirection;
out vec3 vertOutNormal;
out vec3 vertOutNormal2;
//out vec3 reflcoord;

void main()
{
  vertOutPosition = vec3(matModel * vec4(position, 1.0f));

  gl_Position = matModelViewProjection * vec4(position, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord0 = texCoord0;

  vec3 fvObjectPosition = (matModelView * vec4(position, 1.0)).xyz;

  ViewDirection  = fvEyePosition - fvObjectPosition;
  vec3 ViewDirectionn=normalize(ViewDirection);
  LightDirection = fvLightPosition - fvObjectPosition;
  vertOutNormal = matNormal * normal;
  vertOutNormal2 = mat3(transpose(inverse(matModel))) * normal;

  //vec3 fin = ViewDirectionn-(2*(dot(normalize(vertOutNormal2),ViewDirectionn)) * vertOutNormal2);
  //float p = sqrt(pow(fin.x,2)+pow(fin.y,2)+pow((fin.z+1),2));

  //reflcoord = (fin / (2.0 * p)) + 0.5;
}

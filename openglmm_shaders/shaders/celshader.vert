#version 330

// http://sunandblackcat.com/tipFullView.php?l=eng&topicid=15

uniform mat4 matViewProjection;
uniform mat4 matModel;
uniform mat3 matNormal;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

// inputs for fragment shader
smooth out vec3 vertOutNormal;
smooth out vec2 vertOutTexCoord0;
out vec3 vertOutDirectionToLight;
out vec3 vertOutDirectionToCamera;

void main()
{
  vec4 worldPos = matModel * vec4(position, 1.0);

  vertOutNormal = matNormal * normal;
  vertOutTexCoord0 = texCoord0;

  vec3 vectorToLight = lightPosition - worldPos.xyz;
  vertOutDirectionToLight = normalize(vectorToLight);
  vertOutDirectionToCamera = normalize(cameraPosition - worldPos.xyz);

  gl_Position = matViewProjection * worldPos;
}

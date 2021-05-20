#version 330 core

// http://prideout.net/blog/?p=22

uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

smooth out vec3 vertOutNormal;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0);

  vertOutNormal = matNormal * normal;
}

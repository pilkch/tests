#version 330

uniform mat4 matModelViewProjection;

uniform float fOffset; // offset along normal

#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;

void main()
{
  // Offset the position by fOffset along the normal
  gl_Position = matModelViewProjection * vec4(position + (normal * fOffset), 1.0);
}

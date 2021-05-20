#version 330 core

precision highp float;

#include <math.header>

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2DRect texUnit1; // Depth buffer

uniform float fNear;
uniform float fFar;

uniform vec3 diffuseColour;

uniform vec3 ambientColour;
uniform vec3 skyColour; // Add some light towards the top of the particle

struct cSun
{
  vec3 position;
  vec3 colour;
};

uniform cSun sun;

const float fTolerance = 0.01;

smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColour;

void main()
{
  vec2 texCoord1 = gl_FragCoord.xy;
  float fDepthMapDepth = -texture(texUnit1, texCoord1).r;
  float fFragmentDepth = -gl_FragCoord.z;
  if (LinearDepth(fNear, fFar, fFragmentDepth) < LinearDepth(fNear, fFar, fDepthMapDepth)) discard;

  vec4 albedo = texture(texUnit0, vertOutTexCoord0);

  fragmentColour = vec4(diffuseColour, 1.0) * albedo;
}

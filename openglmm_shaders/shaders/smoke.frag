#version 330

precision highp float;

#include <math.header>
#include <lighting.header>

// Using a similar technique to Brutal Legend
// http://drewskillman.com/GDC2010_VFX.pdf

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2DRect texUnit1; // Depth buffer

uniform float fNear;
uniform float fFar;

uniform vec3 ambientColour;
uniform vec3 skyColour; // Add some light towards the top of the particle

uniform vec3 lightColour;
uniform vec3 colour;

struct cSun
{
  vec3 position;
  vec3 colour;
};

uniform cSun sun;

smooth in vec2 vertOutTexCoord0;

smooth in vec3 N;
smooth in vec3 L;

const float fTolerance = 0.01;

out vec4 fragmentColour;

void main()
{
  vec4 albedo = texture(texUnit0, vertOutTexCoord0);
  if (albedo.a < fTolerance) discard;

  vec2 texCoord1 = gl_FragCoord.xy;
  float fDepthMapDepth = -texture(texUnit1, texCoord1).r;
  float fFragmentDepth = -gl_FragCoord.z;
  float fFadeDepth = 0.001f;
  if ((LinearDepth(fNear, fFar, fFragmentDepth) + fFadeDepth) < LinearDepth(fNear, fFar, fDepthMapDepth)) discard;

  //float fDiscard = (LinearDepth(fNear, fFar, fFragmentDepth) < LinearDepth(fNear, fFar, fDepthMapDepth)) ? 1.0 : 0.0;

  // TODO: There are still some bugs where particles are not culled if they are about 5 meters from the camera and behind an opaque object
  // It may be due to one of these:
  // 1) Non-linear depth buffer coordinates
  // 2) Incorrectly checking a linear depth against a non-linear depth, so it kind of works

  // Soft particle edges (Fixes the hard cut that happens when a particle intersects an opaque polygon)
  // http://www.informatik.uni-oldenburg.de/~trigger/page7.html
  // http://blog.wolfire.com/2010/04/Soft-Particles
  // http://developer.download.nvidia.com/whitepapers/2007/SDK10/SoftParticles_hi.pdf
  float fScale = 20.0f;
  float fSoftness = 1.0 - clamp((fDepthMapDepth - fFragmentDepth) * fScale, 0.0, 1.0);

  // Lambert lighting
  vec3 ambient = ambientColour;
  float fLambertDiffuse = CalculateLambertDiffuse(normalize(L), normalize(N));
  vec3 diffuse = lightColour * fLambertDiffuse;

  fragmentColour = vec4((ambient + diffuse) * colour * albedo.rgb, albedo.a * fSoftness);
  //fragmentColour = vec4((ambient + diffuse) * colour * albedo.rgb, 1.0 + 0.0001 * (albedo.a + fSoftness));
  //fragmentColour = vec4(vec3(fDiscard, 1.0 - fDiscard, 1.0 - fSoftness) + 0.001 * ((ambient + diffuse) * colour * albedo.rgb), albedo.a * fSoftness);
}

#version 330

/*******************************************************************************
  Copyright (C) 2013 John Chapman

  This software is distributed freely under the terms of the MIT License.
  See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

uniform sampler2DRect texUnit0;

#define texSceneRTT texUnit0

#define MAX_BLUR_RADIUS 4096

uniform int uBlurRadius = 16;
uniform vec2 uBlurDirection = vec2(1.0, 0.0); // (1,0)/(0,1) for x/y pass

smooth in vec2 vertOutTexCoord;

layout(location=0) out vec4 outFragmentColour;

/*----------------------------------------------------------------------------*/
/*  Incremental, forward-differencing Gaussian elimination based on:
  http://http.developer.nvidia.com/GPUGems3/gpugems3_ch40.html */
vec4 incrementalGauss1D(
  in sampler2DRect srcTex,
  in vec2 srcTexelSize,
  in vec2 origin,
  in int radius,
  in vec2 direction
) {

  int nSamples = clamp(radius, 1, int(MAX_BLUR_RADIUS)) / 2;

  if (nSamples == 0) return texture(srcTex, origin);

  float SIGMA = float(uBlurRadius) / 8.0;
  float sig2 = SIGMA * SIGMA;

  const float TWO_PI  = 6.2831853071795;
  const float E      = 2.7182818284590;

  //  set up incremental counter:
  vec3 gaussInc;
  gaussInc.x = 1.0 / (sqrt(TWO_PI) * SIGMA);
  gaussInc.y = exp(-0.5 / sig2);
  gaussInc.z = gaussInc.y * gaussInc.y;

  // NOTE: We use rectangular textures but this shader was designed for square textures, so we need to adjust the coordinates and then change them back when we actually sample the texture
  vec2 texCoord0To1 = origin / textureSize(srcTex, 0);

  //  accumulate results:
  vec4 result = texture(srcTex, origin) * gaussInc.x;
  for (int i = 1; i < nSamples; ++i) {
    gaussInc.xy *= gaussInc.yz;

    vec2 offset = float(i) * direction * srcTexelSize;
    result += texture(srcTex, textureSize(srcTex, 0) * (texCoord0To1 - offset)) * gaussInc.x;
    result += texture(srcTex, textureSize(srcTex, 0) * (texCoord0To1 + offset)) * gaussInc.x;
  }

  return result;
}

/*----------------------------------------------------------------------------*/
void main()
{
  vec2 texelSize = 1.0 / vec2(textureSize(texSceneRTT, 0));
  outFragmentColour = incrementalGauss1D(texSceneRTT, texelSize, vertOutTexCoord, uBlurRadius, uBlurDirection);
}
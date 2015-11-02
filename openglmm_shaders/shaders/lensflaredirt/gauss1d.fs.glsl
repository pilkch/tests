#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

uniform sampler2D texUnit0;

#define uInputTex texUnit0

#define MAX_BLUR_RADIUS 4096

uniform int uBlurRadius = 16;
uniform float uBlurSigma = 2.0f;
uniform vec2 uBlurDirection = vec2(1.0, 0.0); // (1,0)/(0,1) for x/y pass

noperspective in vec2 vTexcoord;

layout(location=0) out vec4 fResult;

/*----------------------------------------------------------------------------*/
/*	Incremental, forward-differencing Gaussian elimination based on:
	http://http.developer.nvidia.com/GPUGems3/gpugems3_ch40.html */
vec4 incrementalGauss1D(
	in sampler2D srcTex, 
	in vec2 srcTexelSize, 
	in vec2 origin,
	in int radius,
	in vec2 direction
) {

	int nSamples = clamp(radius, 1, int(MAX_BLUR_RADIUS)) / 2;
	
  if (nSamples == 0) return texture(srcTex, origin);
	
	float SIGMA = float(uBlurRadius) / 8.0;
	float sig2 = SIGMA * SIGMA;

	const float TWO_PI	= 6.2831853071795;
	const float E			= 2.7182818284590;

  //	set up incremental counter:
	vec3 gaussInc;
	gaussInc.x = 1.0 / (sqrt(TWO_PI) * SIGMA);
	gaussInc.y = exp(-0.5 / sig2);
	gaussInc.z = gaussInc.y * gaussInc.y;
	
  //	accumulate results:
	vec4 result = texture(srcTex, origin) * gaussInc.x;	
	for (int i = 1; i < nSamples; ++i) {
		gaussInc.xy *= gaussInc.yz;
		
		vec2 offset = float(i) * direction * srcTexelSize;
		result += texture(srcTex, origin - offset) * gaussInc.x;
		result += texture(srcTex, origin + offset) * gaussInc.x;
	}
	
	return result;
}

/*----------------------------------------------------------------------------*/
void main()
{
  vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
  fResult = incrementalGauss1D(uInputTex, texelSize, vTexcoord, uBlurRadius, uBlurDirection); 
}
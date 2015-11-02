#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

#include <../math.header>

uniform sampler2D texUnit0;
uniform sampler2D texUnit1;
uniform sampler2D texUnit2;
uniform sampler2D texUnit3;

#define uInputTex     texUnit0
#define uLensFlareTex texUnit1
#define uLensDirtTex  texUnit2
#define uLensStarTex  texUnit3

uniform mat3 uLensStarMatrix; // camera rotation metric for starburst

noperspective in vec2 vTexcoord;

layout(location=0) out vec4 outFragmentColour;

void main()
{
  // For debugging
  //outFragmentColour = texture(uLensFlareTex, vTexcoord) * lensMod;


  vec4 lensMod = texture(uLensDirtTex, vTexcoord);
  vec2 lensStarTexcoord = (uLensStarMatrix * vec3(vTexcoord, 1.0)).xy;
  lensMod += texture(uLensStarTex, lensStarTexcoord);

  vec4 result = texture(uInputTex, vTexcoord);

  result += texture(uLensFlareTex, vTexcoord) * lensMod;

  outFragmentColour = result;
}
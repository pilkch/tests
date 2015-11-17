#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

uniform sampler2DRect texUnit0;
uniform sampler2DRect texUnit1;
uniform sampler2DRect texUnit2;
uniform sampler2D texUnit3;

#define uInputTex     texUnit0
#define uLensFlareTex texUnit1
#define uLensDirtTex  texUnit2
#define uLensStarTex  texUnit3

uniform mat3 matCameraLensStarBurst; // camera rotation matrix for starburst

smooth in vec2 vertOutTexCoord;

layout(location = 0) out vec4 outFragmentColour;

void main()
{
  vec2 vertOutTexCoord0To1 = vertOutTexCoord / textureSize(uInputTex, 0);

  vec2 vertOutTexCoord0 = vertOutTexCoord;
  vec2 vertOutTexCoord1 = vertOutTexCoord0To1 * textureSize(uLensFlareTex, 0);
  vec2 vertOutTexCoord2 = vertOutTexCoord0To1 * textureSize(uLensDirtTex, 0);
  vec2 vertOutTexCoord3 = vertOutTexCoord0To1 * textureSize(uLensStarTex, 0);

  vec4 lensMod = texture(uLensDirtTex, vertOutTexCoord3);

  vec2 lensStarTexcoord = (matCameraLensStarBurst * vec3(vertOutTexCoord0To1, 1.0)).xy;
  lensMod += texture(uLensStarTex, vertOutTexCoord0To1 * lensStarTexcoord);

  outFragmentColour = texture(uInputTex, vertOutTexCoord) + (texture(uLensFlareTex, vertOutTexCoord1) * lensMod);
}
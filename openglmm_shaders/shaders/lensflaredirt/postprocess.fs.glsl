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

uniform mat4 matCameraLensStarBurst; // Camera rotation matrix for starburst
uniform float fSceneMix0To1;
uniform float fFlareMix0To1;

smooth in vec2 vertOutTexCoord;

layout(location = 0) out vec4 outFragmentColour;

void main()
{
  vec2 vertOutTexCoord0To1 = vertOutTexCoord / textureSize(uInputTex, 0);

  vec2 vertOutTexCoord0 = vertOutTexCoord;
  vec2 vertOutTexCoord1 = vertOutTexCoord0To1 * textureSize(uLensFlareTex, 0);
  vec2 vertOutTexCoord2 = vertOutTexCoord0To1 * textureSize(uLensDirtTex, 0);

  vec2 lensStarTexcoord = (matCameraLensStarBurst * vec4(vertOutTexCoord0To1, 1.0, 1.0)).xy;

  vec4 sceneColour = texture(uInputTex, vertOutTexCoord);
  vec4 flareColour = texture(uLensFlareTex, vertOutTexCoord1) * (texture(uLensDirtTex, vertOutTexCoord2) + texture(uLensStarTex, lensStarTexcoord));

  outFragmentColour = (fSceneMix0To1 * sceneColour) + (fFlareMix0To1 * flareColour);
}
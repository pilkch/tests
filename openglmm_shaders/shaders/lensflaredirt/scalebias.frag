#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

uniform sampler2DRect texUnit0;

#define uInputTex texUnit0

uniform vec3 uScale;
uniform vec3 uBias;

smooth in vec2 vertOutTexCoord;

out vec4 outFragmentColour;

void main()
{
  outFragmentColour = max(vec4(0.0), texture(uInputTex, vertOutTexCoord) + vec4(uBias, 1.0)) * vec4(uScale, 1.0);
}

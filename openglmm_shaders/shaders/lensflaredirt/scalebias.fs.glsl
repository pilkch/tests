#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

uniform sampler2D texUnit0;

#define uInputTex texUnit0

uniform vec4 uScale = vec4(1.0);
uniform vec4 uBias = vec4(0.0);

noperspective in vec2 vTexcoord;

layout(location=0) out vec4 fResult;

/*----------------------------------------------------------------------------*/
void main() {
	fResult = max(vec4(0.0), texture(uInputTex, vTexcoord) + uBias) * uScale;
}
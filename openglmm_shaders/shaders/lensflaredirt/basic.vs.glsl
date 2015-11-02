#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

layout(location=0) in vec3 aPosition;

noperspective out vec2 vTexcoord;

void main()
{
  vTexcoord = aPosition.xy * 0.5 + 0.5;
  gl_Position = vec4(aPosition.xy, 0.0, 1.0);
}
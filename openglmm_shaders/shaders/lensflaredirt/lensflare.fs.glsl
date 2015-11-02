#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

uniform sampler2D texUnit0;
uniform sampler1D texUnit1;

#define uInputTex texUnit0
#define uLensColor texUnit1

const int MAX_SAMPLES = 64;
uniform int uSamples = 8;

uniform float uDispersal = 0.25;
uniform float uHaloWidth = 1.0;
uniform float uDistortion = 1.0;

noperspective in vec2 vTexcoord;

layout(location=0) out vec4 fResult;

/*----------------------------------------------------------------------------*/
vec4 textureDistorted(
	in sampler2D tex, 
	in vec2 texcoord, 
	in vec2 direction,
	in vec3 distortion 
) {
	return vec4(
		texture(tex, texcoord + direction * distortion.r).r,
		texture(tex, texcoord + direction * distortion.g).g,
		texture(tex, texcoord + direction * distortion.b).b,
		1.0
	);
}

/*----------------------------------------------------------------------------*/
void main() {
	vec2 texcoord = -vTexcoord + vec2(1.0); // flip texcoordoords
	vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
	
	vec2 ghostVec = (vec2(0.5) - texcoord) * uDispersal;
	vec2 haloVec = normalize(ghostVec) * uHaloWidth;
	
	vec3 distortion = vec3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);

// sample ghosts:
	vec4 result = vec4(0.0);
	for (int i = 0; i < uSamples; ++i) {
		vec2 offset = fract(texcoord + ghostVec * float(i));
		
		float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
		weight = pow(1.0 - weight, 10.0);
	
		result += textureDistorted(
			uInputTex,
			offset,
			normalize(ghostVec),
			distortion
		) * weight;
	}
	



	result *= texture(uLensColor, length(vec2(0.5) - texcoord) / length(vec2(0.5)));






  //	sample halo:
	float weight = length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));



	weight = pow(1.0 - weight, 10.0);





	result += textureDistorted(
		uInputTex,
		fract(texcoord + haloVec),
		normalize(ghostVec),
		distortion
	) * weight;
	
	fResult = result;
}
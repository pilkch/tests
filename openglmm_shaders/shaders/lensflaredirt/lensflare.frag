#version 330

/*******************************************************************************
	Copyright (C) 2013 John Chapman

	This software is distributed freely under the terms of the MIT License.
	See "license.txt" or "http://copyfree.org/licenses/mit/license.txt".
*******************************************************************************/

uniform sampler2DRect texUnit0;
uniform sampler1D texUnit1;

#define texSceneRTT texUnit0
#define texLensColour texUnit1

const int MAX_SAMPLES = 64;
uniform int uSamples = 8;

uniform float uDispersal = 0.25;
uniform float uHaloWidth = 1.0;
uniform float uDistortion = 1.0;

smooth in vec2 vertOutTexCoord;

layout(location = 0) out vec4 outFragmentColour;

/*----------------------------------------------------------------------------*/
vec4 textureDistorted(
	in sampler2DRect tex, 
	in vec2 texcoord, 
	in vec2 direction,
	in vec3 distortion 
)
{  
  vec2 texCoordRect = textureSize(texSceneRTT, 0);

	return vec4(
		texture(tex, texCoordRect * (texcoord + direction * distortion.r)).r,
		texture(tex, texCoordRect * (texcoord + direction * distortion.g)).g,
		texture(tex, texCoordRect * (texcoord + direction * distortion.b)).b,
		1.0
	);
}

/*----------------------------------------------------------------------------*/
void main()
{
  // NOTE: We use rectangular textures but this shader was designed for square textures, so we need to adjust the coordinates and then change them back inside textureDistorted()
  vec2 texCoord0To1 = vertOutTexCoord / textureSize(texSceneRTT, 0);
  
	//vec2 texcoord = -vertOutTexCoord texCoord0To1 + vec2(1.0); // flip texcoordoords
	vec2 texcoord = -texCoord0To1 + vec2(1.0); // flip texcoordoords
	vec2 texelSize = 1.0 / vec2(textureSize(texSceneRTT, 0));
	
	vec2 ghostVec = (vec2(0.5) - texcoord) * uDispersal;
	vec2 haloVec = normalize(ghostVec) * uHaloWidth;
	
	vec3 distortion = vec3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);

  // Sample ghosts
	vec4 result = vec4(0.0);
	for (int i = 0; i < uSamples; ++i) {
		vec2 offset = fract(texcoord + ghostVec * float(i));
		
		float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
		weight = pow(1.0 - weight, 10.0);
	
		result += textureDistorted(
			texSceneRTT,
			offset,
			normalize(ghostVec),
			distortion
		) * weight;
	}

	result *= texture(texLensColour, length(vec2(0.5) - texcoord) / length(vec2(0.5)));

  // sample halo
	float weight = length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));

	weight = pow(1.0 - weight, 10.0);

	result += textureDistorted(
		texSceneRTT,
		fract(texcoord + haloVec),
		normalize(ghostVec),
		distortion
	) * weight;
	
	outFragmentColour = result;
}

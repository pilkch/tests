#version 330 core

uniform sampler2D windFlowMapTexture; // Wind flow texture

uniform mat4 matModelViewProjection;
uniform mat4 matModel;

uniform float fCurrentTime;
uniform vec3 wind;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec4 colourAndWindInfluence; // NOTE: The rgb parts are the colour, the a value is how much the wind influences this point

// Outputs for fragment program
smooth out vec2 vertOutTexCoord0;
out vec3 vertOutNormal;
smooth out vec3 vertOutColour;

void main()
{
  float fWindInfluence = colourAndWindInfluence.a;

  float majorWindStrength = 0.5;
  float windVariability = 0.00015;
  float minorWindStrength = 0.3;

	vec2 uv = position.xz / 10.0 + vec2(windVariability * fCurrentTime);
	uv.x = mod(uv.x, 1.0);
	uv.y = mod(uv.y, 1.0);
	vec2 flowMapValue = texture(windFlowMapTexture, uv).rg;

  // Normalise the values
  vec2 flowMapValueNormalised = (-vec2(0.5) + flowMapValue.rg) * 2.0;

  vec3 minorWind = minorWindStrength * vec3(flowMapValueNormalised.x, 0.0, flowMapValueNormalised.y);

  vec3 majorWind = majorWindStrength * wind;

  // Add the major and minor wind together
  vec3 combinedWind = fWindInfluence * (majorWind + minorWind);

  gl_Position = matModelViewProjection * vec4(position + combinedWind, 1.0);

  // Pass on the texture coordinates, normal, and colour
  vertOutTexCoord0 = texCoord0;
  vertOutNormal = mat3(transpose(inverse(matModel))) * normal;
  vertOutColour = colourAndWindInfluence.rgb;
}

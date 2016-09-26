#version 330 core

uniform sampler2DRect texUnit0; // Normal scene texture
uniform sampler2DRect texUnit1; // Glow pixels texture
uniform sampler2DRect texUnit2; // Bright pixels texture

uniform int iGlowTextureSize;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 scene = texture(texUnit0, vertOutTexCoord).rgb;
  vec3 glow = texture(texUnit1, vertOutTexCoord / float(iGlowTextureSize)).rgb;
  vec3 brightPixels = texture(texUnit2, vertOutTexCoord).rgb;

  fragmentColour = vec4(scene + 2.0 * (glow + brightPixels), 1.0);
}

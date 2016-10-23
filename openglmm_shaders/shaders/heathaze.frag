#version 330

// Based on https://github.com/SFML/SFML/wiki/Source:-HeatHazeShader
// That version is 2D, so this version basically just applies the same 2D filter wherever there are white pixels in a mask of "hot objects"

precision highp float;

uniform sampler2DRect texUnit0; // Scene texture
uniform sampler2DRect texUnit1; // Scene heat map texture
uniform sampler2D texUnit2; // Noise map texture

uniform vec2 screenSize;

uniform float currentTime; // Time used to scroll the distortion map
uniform float distortionFactor = 0.2f; // Factor used to control severity of the effect
uniform float riseFactor = 0.2f; // Factor used to control how fast air rises

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  float scaledTime = 0.05 * currentTime;

  vec2 texCoordSquare = vertOutTexCoord / screenSize;

  vec2 distortionMapCoordinate = texCoordSquare;

  // We use the current time value to scroll our distortion texture upwards
  // Since we enabled texture repeating, OpenGL takes care of
  // coordinates that lie outside of [0, 1] by discarding
  // the integer part and keeping the fractional part
  // Basically performing a "floating point modulo 1"
  // 1.1 = 0.1, 2.4 = 0.4, 10.3 = 0.3 etc.
  distortionMapCoordinate.t += scaledTime * riseFactor;

  vec2 distortionMapValue = texture(texUnit2, distortionMapCoordinate).xy;

  // The values are normalized by OpenGL to lie in the range [0, 1]
  // We want negative offsets too, so we subtract 0.5 and multiply by 2
  // We end up with values in the range [-1, 1]
  vec2 distortionPositionOffset = distortionMapValue;
  distortionPositionOffset -= vec2(0.5f, 0.5f);
  distortionPositionOffset *= 2.f;

  // The factor scales the offset and thus controls the severity
  distortionPositionOffset *= distortionFactor;

  // Multiply the heat distortion by the heat map 0..1 value
  float heatDistortionFactor = texture(texUnit1, vertOutTexCoord).r;

  vec2 distortedTextureCoordinate = (screenSize * distortionPositionOffset);

  vec3 colour = texture(texUnit0, vertOutTexCoord + (heatDistortionFactor * distortedTextureCoordinate)).rgb;
  
  fragmentColour = vec4(colour, 1.0);
}

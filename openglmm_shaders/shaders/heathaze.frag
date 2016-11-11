#version 330

// Based on https://github.com/SFML/SFML/wiki/Source:-HeatHazeShader
// That version is 2D, so this version basically just applies the same 2D filter wherever there are white pixels in a mask of "hot objects"

precision highp float;

uniform sampler2DRect texUnit0; // Scene texture
uniform sampler2DRect texUnit1; // Scene noise and heat map texture

uniform vec2 screenSize;

const float fNoiseScale = 1.0f;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec4 distortionNoiseAndHeatMap = texture(texUnit1, vertOutTexCoord);

  vec2 distortionPositionOffset = distortionNoiseAndHeatMap.xy;

  // Multiply the heat distortion by the heat map 0..1 value
  float fHeatDistortionFactor = distortionNoiseAndHeatMap.b;

  vec2 distortedTextureCoordinate = fNoiseScale * screenSize * distortionPositionOffset;

  vec3 colour = texture(texUnit0, vertOutTexCoord + (fHeatDistortionFactor * distortedTextureCoordinate)).rgb;

  // Add a little bit of red
  colour.r += 0.5f * fHeatDistortionFactor;

  fragmentColour = vec4(colour, 1.0);
}

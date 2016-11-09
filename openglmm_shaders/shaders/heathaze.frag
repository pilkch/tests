#version 330

// Based on https://github.com/SFML/SFML/wiki/Source:-HeatHazeShader
// That version is 2D, so this version basically just applies the same 2D filter wherever there are white pixels in a mask of "hot objects"

precision highp float;

uniform sampler2DRect texUnit0; // Scene texture
uniform sampler2DRect texUnit1; // Scene noise and heat map texture

uniform vec2 screenSize;

uniform float distortionFactor = 0.2f; // Factor used to control severity of the effect

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;


// TODO: Add blur?

void main()
{
  vec4 distortionNoiseAndHeatMap = texture(texUnit1, vertOutTexCoord);

  vec2 distortionMapValue = distortionNoiseAndHeatMap.xy;

  // The values are normalized by OpenGL to lie in the range [0, 1]
  // We want negative offsets too, so we subtract 0.5 and multiply by 2
  // We end up with values in the range [-1, 1]
  vec2 distortionPositionOffset = distortionNoiseAndHeatMap.xy;
  //distortionPositionOffset -= vec2(0.5f, 0.5f);
  //distortionPositionOffset *= 2.f;

  // The factor scales the offset and thus controls the severity
  distortionPositionOffset *= distortionFactor;

  // Multiply the heat distortion by the heat map 0..1 value
  float heatDistortionFactor = distortionNoiseAndHeatMap.a;

  vec2 distortedTextureCoordinate = (screenSize * distortionPositionOffset);

  vec3 colour = texture(texUnit0, vertOutTexCoord + (heatDistortionFactor * distortedTextureCoordinate)).rgb;

  fragmentColour = vec4(colour, 1.0);
}

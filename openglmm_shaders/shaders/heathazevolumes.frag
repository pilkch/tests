#version 330 core

uniform sampler2D texUnit0; // Noise map texture

uniform float currentTime; // Time used to scroll the distortion map

uniform float riseFactor = 0.2f; // Factor used to control how fast air rises

smooth in vec2 vertOutTexCoord;
smooth in float vertOutHeatValue;

out vec4 fragmentColour;

void main()
{
  float scaledTime = 0.03 * currentTime;

  vec2 distortionMapCoordinate = vertOutTexCoord;

  // We use the current time value to scroll our distortion texture upwards
  // Since we enabled texture repeating, OpenGL takes care of
  // coordinates that lie outside of [0, 1] by discarding
  // the integer part and keeping the fractional part
  // Basically performing a "floating point modulo 1"
  // 1.1 = 0.1, 2.4 = 0.4, 10.3 = 0.3 etc.
  distortionMapCoordinate.t += scaledTime * riseFactor;

  vec2 distortionMapValue = texture(texUnit0, distortionMapCoordinate).xy;

  // Output the texture coordinates
  // Output alpha from 0 to 1 where 0 is not affected by heat haze and 1 is the hottest object in the scene
  fragmentColour = vec4(distortionMapValue, 0.0f, vertOutHeatValue);
}

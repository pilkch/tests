#version 330 core

uniform sampler2D texUnit0; // Noise map texture

uniform float currentTime; // Time used to scroll the distortion map

uniform float riseFactor = 0.0002f; // Factor used to control how fast air rises

smooth in vec2 vertOutTexCoord;
smooth in float vertOutHeatValue;

out vec4 fragmentColour;

void main()
{
  vec2 distortionMapCoordinate = vertOutTexCoord;

  // We use the current time value to scroll our distortion texture upwards
  distortionMapCoordinate.t += currentTime * riseFactor;

  vec2 distortionMapValue = texture(texUnit0, distortionMapCoordinate).xy;

  distortionMapValue -= vec2(0.5f, 0.5f);
  distortionMapValue *= 2.f;

  // Output the texture coordinates
  // Output alpha from 0 to 1 where 0 is not affected by heat haze and 1 is the hottest object in the scene
  fragmentColour = vec4(distortionMapValue, vertOutHeatValue, 1.0f);
}

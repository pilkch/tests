#version 330 core

smooth in float vertOutHeatValue;

out vec4 fragmentColour;

void main()
{
  // Output a black and white pixel where black is not affected by heat haze and white is the hottest object in the scene
  fragmentColour = vec4(vertOutHeatValue, vertOutHeatValue, vertOutHeatValue, 1.0);
}

#version 330 core

uniform vec3 colour;

out vec4 fragmentColour;

void main()
{
  fragmentColour = vec4(colour, 1.0);
}

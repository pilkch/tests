#version 330 core

uniform vec4 colour = vec4(0.5, 0.5, 0.5, 0.5);

out vec4 fragmentColour;

void main()
{
  fragmentColour = colour;
}

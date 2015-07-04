#version 330

uniform vec4 colour;

out vec4 fragmentColour;

void main()
{
  fragmentColour = colour;
}

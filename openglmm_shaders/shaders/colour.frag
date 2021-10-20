#version 330 core

uniform vec4 colour;

out vec4 fragmentColour;

void main()
{
  fragmentColour = colour;
}

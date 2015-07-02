#version 330

uniform vec3 colour;

out vec4 fragmentColour;

void main()
{
  fragmentColour = vec4(colour, 1.0);
}

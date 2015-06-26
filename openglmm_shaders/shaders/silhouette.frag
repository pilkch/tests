#version 330

uniform vec3 colour;

out vec4 fragmentColor;

void main()
{
  fragmentColor = vec4(colour, 1.0);
}

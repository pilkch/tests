#version 330 core

in vec3 vertOutColour;

out vec4 fragmentColour;

void main()
{
  fragmentColour = vec4(vertOutColour, 1.0f);
}


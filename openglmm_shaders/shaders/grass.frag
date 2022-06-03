#version 330 core

in vec3 vertOutNormal;
smooth in vec3 vertOutColour;

const float tolerance = 0.2;

out vec4 fragmentColour;

void main()
{
  fragmentColour = vec4(vertOutColour.rgb, 1.0);
}



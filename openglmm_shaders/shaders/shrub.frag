
#version 330 core

uniform sampler2D texUnit0; // Base texture

smooth in vec2 vertOutTexCoord0;
in vec3 vertOutNormal;
smooth in vec3 vertOutColour;

const float tolerance = 0.2;

out vec4 fragmentColour;

void main()
{
  vec4 colour = texture(texUnit0, vertOutTexCoord0);
  if (colour.a < tolerance) discard;

  fragmentColour = vec4(colour.rgb * vertOutColour, 1.0);
}

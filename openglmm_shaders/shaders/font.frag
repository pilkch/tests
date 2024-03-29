#version 330 core

uniform sampler2D texUnit0; // Base texture

const float tolerance = 0.8;

smooth in vec4 vertOutColour;
smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColour;

void main()
{
  vec4 diffuse = texture2D(texUnit0, vertOutTexCoord0);
  if (diffuse.a < tolerance) discard;

  // TODO: Test with alpha = 1.0
  fragmentColour = vec4(diffuse.rgb * vertOutColour.rgb, diffuse.a);
}

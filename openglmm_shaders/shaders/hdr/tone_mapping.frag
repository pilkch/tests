#version 330 core

uniform sampler2DRect texUnit0;

uniform float fMaxRGBValue;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 Color = texture(texUnit0, vertOutTexCoord).rgb;

  fragmentColour = vec4(Color / fMaxRGBValue, 1.0);
}

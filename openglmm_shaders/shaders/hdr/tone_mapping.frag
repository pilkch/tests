#version 330

uniform sampler2DRect texUnit0;

uniform float MaxRGBValue;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 Color = texture(texUnit0, vertOutTexCoord).rgb;

  fragmentColour = vec4(Color / MaxRGBValue, 1.0);
}

#version 330

uniform sampler2DRect texUnit0;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 Color = texture(texUnit0, vertOutTexCoord).rgb;

  float MaxRGBValue = max(Color.r, max(Color.g, Color.b));
  if (MaxRGBValue < 0.9) Color = vec3(0.0, 0.0, 0.0);

  fragmentColour = vec4(Color, 1.0);
}

#version 330

uniform sampler2DRect texUnit0;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 Color = texture(texUnit0, vertOutTexCoord).rgb;

  float MaxRGBValue = max(Color.r, max(Color.g, Color.b));

  fragmentColour = vec4(MaxRGBValue, MaxRGBValue, MaxRGBValue, 1.0);
}

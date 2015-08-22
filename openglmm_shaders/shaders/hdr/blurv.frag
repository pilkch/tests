#version 330

uniform sampler2DRect texUnit0;

uniform int Width;
uniform float odh;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 Color = vec3(0.0);
  int wp1 = Width + 1;
  float Sum = 0.0;

  for (int y = -Width; y <= Width; y++) {
    float width = (wp1 - abs(float(y)));
    Color += texture(texUnit0, vertOutTexCoord + vec2(0.0, odh * y)).rgb * width;
    Sum += width;
  }

  fragmentColour = vec4(Color / Sum, 1.0);
}

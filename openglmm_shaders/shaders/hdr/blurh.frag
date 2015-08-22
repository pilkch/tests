#version 330

uniform sampler2DRect texUnit0;

uniform int Width;
uniform float odw;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 Color = vec3(0.0);
  int wp1 = Width + 1;
  float Sum = 0.0;

  for (int x = -Width; x <= Width; x++) {
    float width = (wp1 - abs(float(x)));
    Color += texture(texUnit0, vertOutTexCoord + vec2(odw * x, 0.0)).rgb * width;
    Sum += width;
  }

  fragmentColour = vec4(Color / Sum, 1.0);
}

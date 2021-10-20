#version 330 core

uniform sampler2DRect texUnit0;

uniform float odx;
uniform float ody;

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  float a = texture(texUnit0, vertOutTexCoord + vec2(0.0, 0.0)).r;
  float b = texture(texUnit0, vertOutTexCoord + vec2(odx, 0.0)).r;
  float c = texture(texUnit0, vertOutTexCoord + vec2(odx, ody)).r;
  float d = texture(texUnit0, vertOutTexCoord + vec2(0.0, ody)).r;

  float maxlum = max(max(a, b), max(c, d));

  fragmentColour = vec4(maxlum, maxlum, maxlum, 1.0);
}

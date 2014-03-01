#version 140
// 140 adds sample2DRect

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColor;

void main()
{
  vec4 color = texture(texUnit0, vertOutTexCoord);

  color.g += 0.2;

  fragmentColor = color;
}

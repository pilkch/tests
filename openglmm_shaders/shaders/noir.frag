#version 140
// 140 adds sample2DRect

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColor;

void main()
{
  vec4 color = texture(texUnit0, vertOutTexCoord);

  if ((color.r > 0.4) && (color.b < 0.1) && (color.g < 0.1)) {
    // Red colour is dominant, emphasise it
    color.r = 1.0;
    color.b = 0.0;
    color.g = 0.0;
  } else {
    // Greyscale
    float fSin = max(color.r, max(color.g, color.b)) / 2 + min(color.r, min(color.g, color.b)) / 2;

    if (fSin < 0.1 || fSin >0.95 ) {
      color.r = 0.0;
      color.b = 0.0;
      color.g = 0.0;
    } else {
      color.r = 1.0;
      color.b = 1.0;
      color.g = 1.0;
    }
  }

  fragmentColor = color;
}

#version 130

uniform sampler2D texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColor;

void main()
{
  fragmentColor = texture2D(texUnit0, vertOutTexCoord);
}

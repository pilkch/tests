#version 130

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Detail texture

smooth in vec2 vertOutTexCoord0;
smooth in vec2 vertOutTexCoord1;

out vec4 fragmentColor;

void main()
{
  vec4 diffuse = texture2D(texUnit0, vertOutTexCoord0);
  vec4 detail = texture2D(texUnit1, vertOutTexCoord1);

  fragmentColor = vec4((diffuse * detail).rgb, 1.0);
}

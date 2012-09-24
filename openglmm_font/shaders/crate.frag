#version 130

uniform sampler2D texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColor;

void main()
{
  vec4 diffuse = texture2D(texUnit0, vertOutTexCoord0);

  fragmentColor = vec4(diffuse.rgb, 1.0);
}

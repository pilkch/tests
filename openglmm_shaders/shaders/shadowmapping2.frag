#version 330

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Lightmap texture

smooth in vec2 vertOutTexCoord0;
smooth in vec2 vertOutTexCoord1;

out vec4 fragmentColour;

void main()
{
  vec4 diffuse = texture2D(texUnit0, vertOutTexCoord0);
  vec4 lightmap = texture2D(texUnit1, vertOutTexCoord1);

  fragmentColour = vec4((diffuse * lightmap).rgb, 1.0);
}

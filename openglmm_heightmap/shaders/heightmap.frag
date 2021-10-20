#version 330 core

uniform mat4 matModelViewProjection;

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Lightmap texture
uniform sampler2D texUnit2; // Detail texture

smooth in vec2 vertOutTexCoord0;
smooth in vec2 vertOutTexCoord1;
smooth in vec2 vertOutTexCoord2;

out vec4 fragmentColour;

void main(void)
{
  vec4 diffuse = texture(texUnit0, vertOutTexCoord0);
  vec4 lightmap = texture(texUnit1, vertOutTexCoord1);
  vec4 detail = texture(texUnit2, vertOutTexCoord2);

  fragmentColour = vec4((diffuse * lightmap * detail).rgb, 1.0);
}

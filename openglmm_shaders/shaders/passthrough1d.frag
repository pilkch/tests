#version 330

uniform sampler1D texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  fragmentColour = texture(texUnit0, vertOutTexCoord.x);
}

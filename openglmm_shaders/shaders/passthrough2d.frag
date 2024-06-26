#version 330 core

uniform sampler2D texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  fragmentColour = texture(texUnit0, vertOutTexCoord);
}

#version 330 core

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  // vec2(1.0, 1.0) was passed in by the texture coordinates, so multiply by the texture size
  vec2 textureDimensions = textureSize(texUnit0);

  fragmentColour = texture(texUnit0, textureDimensions * vertOutTexCoord);
}

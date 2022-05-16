#version 330 core

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  // TODO: Fix this
  //vec2 textureDimensions = textureSize(texUnit0);
  vec2 textureDimensions = vec2(1.0, 1.0);

  fragmentColour = texture(texUnit0, textureDimensions * vertOutTexCoord);
}

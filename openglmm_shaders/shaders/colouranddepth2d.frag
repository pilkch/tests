#version 330 core

uniform sampler2DRect texUnit0; // Diffuse texture
uniform sampler2DRect texUnit1; // Depth texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec2 textureDimensions = textureSize(texUnit0);

  // Output the colour
  fragmentColour = texture(texUnit0, textureDimensions * vertOutTexCoord);

  // Output the depth
  gl_FragDepth = texture(texUnit1, textureDimensions * vertOutTexCoord).r;
}

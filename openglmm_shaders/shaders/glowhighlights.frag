#version 330 core

uniform sampler2D texUnit0; // Black texture with white pixels that should be highlights

uniform vec3 glowColour;

smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColour;

void main()
{
  vec3 colour = texture(texUnit0, vertOutTexCoord0).rgb;

  // Mixing the glow colour with the texture colour will pick out and change the colour of white pixels
  fragmentColour = vec4(glowColour * colour, 1.0);
}

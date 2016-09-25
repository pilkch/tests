#version 330 core

uniform sampler2DRect texUnit0; // Normal scene texture
uniform sampler2DRect texUnit1; // Glow pixels texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 scene = texture(texUnit0, vertOutTexCoord).rgb;
  vec3 glow = texture(texUnit1, vertOutTexCoord).rgb;

  fragmentColour = vec4(scene + glow, 1.0);
}

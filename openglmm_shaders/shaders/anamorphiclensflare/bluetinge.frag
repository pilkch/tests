#version 330 core

uniform sampler2DRect texUnit0; // Scene texture
uniform sampler2DRect texUnit1; // Final bright pixels texture

uniform vec2 brightPixelsStretchedScale; // The scale of the final bright pixels texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  vec3 scene = texture(texUnit0, vertOutTexCoord).rgb;
  vec3 brightPixels = texture(texUnit1, brightPixelsStretchedScale * vertOutTexCoord).rgb;

  const vec3 blueTinge = vec3(0.6, 0.6, 1.0);

  fragmentColour = vec4(scene + (brightPixels * blueTinge), 1.0);
}

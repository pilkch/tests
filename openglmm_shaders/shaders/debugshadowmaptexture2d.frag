#version 330 core

uniform sampler2DShadow texUnit0; // Shadow texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  // TODO: Use normal 0.0..1.0 coordinates so that we don't have to divide by a rectangle
  //const vec2 scale = vec2(1.0 / 1024.0);
  //const vec2 scale = vec2(1.0 / 1280.0, 1.0 / 720.0);
  //vec2 scale = textureSize(texUnit0, 0);
  vec2 scale = vec2(1.0, 1.0);

  const float fDepth = 1.0;
  const float epsilon = 0.00001;

  // fShadowFactor is now a floating-point value between 0 (fully blocked) and 1 (fully unblocked)
  float fShadowFactor = texture(texUnit0, vec3(scale * vertOutTexCoord, fDepth + epsilon));

  fragmentColour = vec4(vec3(fShadowFactor, 0.0, 0.0), 1.0);
}

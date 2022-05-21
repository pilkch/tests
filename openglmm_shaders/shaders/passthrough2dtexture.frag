#version 330 core

uniform sampler2D texUnit0; // Diffuse texture

uniform vec3 fillColour = vec3(1.0f, 1.0f, 1.0f);

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  float alpha = texture(texUnit0, vertOutTexCoord).a;
  if (alpha < 0.1f) discard;

  fragmentColour = vec4(fillColour, 1.0);
}

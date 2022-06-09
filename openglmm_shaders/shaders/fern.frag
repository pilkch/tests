#version 330 core

uniform sampler2D texUnit0; // Diffuse texture

in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;

const float tolerance = 0.2;

out vec4 fragmentColour;

void main()
{
  vec4 albedo = texture(texUnit0, vertOutTexCoord0);
  if (albedo.a < tolerance) discard;

  fragmentColour = vec4(albedo.rgb, 1.0);
}

#version 330 core

uniform sampler2DRect texUnit0; // Diffuse texture

in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;
smooth in vec3 vertOutColour;

const float tolerance = 0.2;

out vec4 fragmentColour;

void main()
{
  vec2 textureDimensions = textureSize(texUnit0);

  vec4 albedo = texture(texUnit0, textureDimensions * vertOutTexCoord0);
  if (albedo.a < tolerance) discard;

  fragmentColour = vec4(albedo.rgb * vertOutColour.rgb, 1.0);
}

#version 330

uniform sampler2D texUnit0; // Diffuse texture

uniform vec3 diffuseColour;

uniform vec3 ambientColour;
uniform vec3 skyColour; // Add some light towards the top of the particle

struct cSun
{
  vec3 position;
  vec3 colour;
};

uniform cSun sun;

smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColour;

void main()
{
  vec4 albedo = texture(texUnit0, vertOutTexCoord0);

  fragmentColour = vec4(diffuseColour, 1.0) * albedo;
}

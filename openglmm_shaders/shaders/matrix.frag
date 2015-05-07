#version 330

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColor;

vec3 desaturate(vec3 color, float amount)
{
  vec3 greyscale = vec3(dot(vec3(0.2126, 0.7152, 0.0722), color));
  return vec3(mix(color, greyscale, amount));
}

void main()
{
  vec4 color = texture(texUnit0, vertOutTexCoord);

  color.rgb = desaturate(color.rgb, 0.1);

  color.g += 0.2;

  fragmentColor = color;
}

#version 330

// http://sunandblackcat.com/tipFullView.php?l=eng&topicid=15

uniform sampler2D texUnit0; // Diffuse texture

uniform vec3 colour; // Shading color
uniform float nShades;

smooth in vec3 vertOutNormal;
smooth in vec2 vertOutTexCoord0;

in vec3 vertOutDirectionToLight;
in vec3 vertOutDirectionToCamera;

out vec4 fragmentColor;

float CalculateDiffuseSimple(vec3 L, vec3 N)
{
  return clamp(dot(L, N), 0.0, 1.0);
}

float CalculateSpecularSimple(vec3 L, vec3 N, vec3 H)
{
  if (dot(N, L) > 0.0) return pow(clamp(dot(H, N), 0.0, 1.0), 64.0);

  return 0.0;
}

void main()
{
  // Sample color from diffuse texture
  vec3 colfromtex = texture2D(texUnit0, vertOutTexCoord0).rgb;

  // Calculate total intensity of lighting
  vec3 halfVector = normalize(vertOutDirectionToLight + vertOutDirectionToCamera);
  float iambi = 0.1;
  float idiff = CalculateDiffuseSimple(vertOutDirectionToLight, vertOutNormal);
  float ispec = CalculateSpecularSimple(vertOutDirectionToLight, vertOutNormal, halfVector);
  float intensity = iambi + idiff + ispec;

  // Quantize intensity for cel shading
  float shadeIntensity = max(ceil(intensity * nShades) / nShades, 1.0);

  fragmentColor = vec4(colour * colfromtex * shadeIntensity, 1.0);
}

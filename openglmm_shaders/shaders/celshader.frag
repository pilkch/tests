#version 330 core

// http://prideout.net/blog/?p=22

uniform vec3 colour; // Shading color
uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 ambientMaterial;
uniform vec3 specularMaterial;
uniform float fShininess;

smooth in vec3 vertOutNormal;

out vec4 fragmentColour;

float stepmix(float edge0, float edge1, float E, float x)
{
  float T = clamp(0.5 * (x - edge0 + E) / E, 0.0, 1.0);
  return mix(edge0, edge1, T);
}

void main()
{
  vec3 N = normalize(vertOutNormal);
  vec3 L = lightPosition;
  vec3 Eye = cameraPosition;
  vec3 H = normalize(L + Eye);

  float df = max(0.0, dot(N, L));
  vec3 Diffuse = colour;

  float sf = max(0.0, dot(N, H));
  sf = pow(sf, fShininess);

  const float A = 0.1;
  const float B = 0.3;
  const float C = 0.6;
  const float D = 1.0;
  float E = fwidth(df);

  if      (df > A - E && df < A + E) df = stepmix(A, B, E, df);
  else if (df > B - E && df < B + E) df = stepmix(B, C, E, df);
  else if (df > C - E && df < C + E) df = stepmix(C, D, E, df);
  else if (df < A) df = 0.0;
  else if (df < B) df = B;
  else if (df < C) df = C;
  else df = D;

  E = fwidth(sf);

  if (sf > 0.5 - E && sf < 0.5 + E) {
    sf = smoothstep(0.5 - E, 0.5 + E, sf);
  } else {
    sf = step(0.5, sf);
  }

  fragmentColour = vec4(ambientMaterial + (df * Diffuse) + (sf * specularMaterial), 1.0);
}

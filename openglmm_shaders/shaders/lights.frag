#version 130

uniform sampler2D texUnit0; // Diffuse texture

struct cLight
{
  vec3 position;
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
};

uniform cLight light;

struct cMaterial
{
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
  float fShininess;
};

uniform cMaterial material;

smooth in vec2 vertOutTexCoord0;

smooth in vec3 out_normal;
smooth in vec3 out_light_half_vector;

out vec4 fragmentColour;


vec3 ApplyLightDirectionalLight()
{
  return vec3(0.0, 0.0, 0.0);
}

vec3 ApplyLightPointLight()
{
  return vec3(0.0, 0.0, 0.0);
}

vec3 ApplyLightSpotLight()
{
  return vec3(0.0, 0.0, 0.0);
}


void main()
{
  vec3 diffuse = texture2D(texUnit0, vertOutTexCoord0).rgb;

  // Calculate the lighting
  vec3 V = vec3 (0.0, 0.0, 1.0);

  // Normalize the vector from surface to light position
  vec3 H = normalize(V + out_light_half_vector);

  vec4 A = material.ambientColour * light.ambientColour;
  float fDiffuse = max(dot(out_normal, out_light_half_vector), 0.0);
  float pf = 0.0;
  if (fDiffuse == 0.0)
  {
    pf = 0.0;
  }
  else
  {
    pf = max(pow(dot(out_normal, H), material.fShininess), 0.0);
  }
  vec4 S = light.specularColour * material.specularColour * pf;
  vec4 D = fDiffuse * material.diffuseColour * light.diffuseColour;

  // Pass on the colour
  vec4 lighting = A + D + S;

  fragmentColour = vec4(diffuse + ApplyLightDirectionalLight() + ApplyLightPointLight() + ApplyLightSpotLight(), 1.0) * lighting;
}
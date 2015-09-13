#version 330 core

// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/

// Ouputs
layout(location = 0) out vec3 fragmentColour;

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2DArrayShadow texUnit1; // Shadow map

in vec3 LightDirection;
in vec3 LightDirectionReflected;
in vec3 CameraDirection;
in vec3 Normal;
in vec4 ShadowTexCoord[5];

void main()
{
  float Shadow = 0.0;

  for (int i = 0; i < 5; i++) {
    Shadow += shadow2DArray(texUnit1, vec4(ShadowTexCoord[i].xy / ShadowTexCoord[i].w, i, ShadowTexCoord[i].z / ShadowTexCoord[i].w)).r;
  }

  Shadow /= 5.0;

  float NdotLD = max(dot(normalize(LightDirection), Normal), 0.0) * Shadow;
  float Spec = pow(max(dot(normalize(LightDirectionReflected), normalize(CameraDirection)), 0.0), 32.0) * Shadow;

  fragmentColour = vec4(texture(texUnit0, gl_TexCoord[0].st).rgb * (0.25 + NdotLD * 0.75 + Spec), 1.0);
}

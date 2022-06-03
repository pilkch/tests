#version 330 core

uniform mat4 matModelViewProjection;
uniform mat4 matModel;

uniform vec3 wind;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec4 colourAndWindInfluence; // NOTE: The rgb parts are the colour, the a value is how much the wind influences this point

// Outputs for fragment program
smooth out vec2 vertOutTexCoord0;
out vec3 vertOutNormal;
smooth out vec3 vertOutColour;

void main()
{
  float fWindInfluence = colourAndWindInfluence.a;
  gl_Position = matModelViewProjection * vec4(position + (fWindInfluence * wind), 1.0);

  // Pass on the texture coordinates, normal, and colour
  vertOutTexCoord0 = texCoord0;
  vertOutNormal = mat3(transpose(inverse(matModel))) * normal;
  vertOutColour = colourAndWindInfluence.rgb;
}

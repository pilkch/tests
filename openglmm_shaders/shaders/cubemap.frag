#version 330

uniform samplerCube texUnit0; // Cubemap texture

uniform vec3 cameraPos;

in vec3 Normal;
in vec3 Position;

smooth in vec2 vertOutTexCoord0;

out vec4 fragmentColour;

void main()
{
  vec3 I = normalize(Position - cameraPos);
  vec3 R = reflect(I, normalize(Normal));
  fragmentColour = texture(texUnit0, R);
}

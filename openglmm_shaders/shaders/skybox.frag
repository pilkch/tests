#version 330 core

uniform samplerCube texUnit0;

in vec3 TexCoords;

out vec4 fragmentColour;

void main()
{
    fragmentColour = vec4(texture(texUnit0, TexCoords).rgb, 1.0);
}

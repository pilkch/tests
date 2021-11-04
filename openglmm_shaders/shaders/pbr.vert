#version 330 core

uniform mat4 matView;
uniform mat4 matProjection;
uniform mat4 matModel;
uniform mat3 matNormal;

uniform vec3 uLightPosition;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 vNormal;
out vec3 vLightPosition;
out vec3 vPosition;

void main()
{
    vec4 worldSpacePosition = matModel * vec4(position, 1.0);
    vec4 viewSpacePosition = matView * worldSpacePosition;

    vNormal = matNormal * normal;
    vLightPosition = (matView * vec4(uLightPosition, 1.0)).xyz;
    vPosition = viewSpacePosition.xyz;

    gl_Position = matProjection * viewSpacePosition;
}

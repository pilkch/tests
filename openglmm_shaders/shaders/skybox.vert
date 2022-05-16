#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 matProjection;
uniform mat4 matView;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;

    vec4 pos = matProjection * matView * vec4(aPos, 1.0);

    // Trick to place the skybox behind any other 3D model
    gl_Position = pos.xyww;
}

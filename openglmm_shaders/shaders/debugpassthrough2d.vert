#version 330

// This is the same as the normal passthrough2d.vert but it has a variable texture size
// It is really just for drawing textures to the screen for debugging purposes
// TODO: Replace this with textureSize(texUnit0, 0) in the fragment shader

uniform mat4 matModelViewProjection;
uniform vec2 textureSize;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord0;

// Texture coordinates for fragment program
smooth out vec2 vertOutTexCoord;

void main()
{
  gl_Position = matModelViewProjection * vec4(position, 1.0, 1.0);

  // Pass on the texture coordinates
  vertOutTexCoord = textureSize * texCoord0;
}

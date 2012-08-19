uniform mat4 matModelViewProjection;

void main()
{
  gl_Position = matModelViewProjection * gl_Vertex;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_TextureMatrix[0] * gl_MultiTexCoord1;
}



void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_TextureMatrix[0] * gl_MultiTexCoord1;
  gl_TexCoord[2] = gl_TextureMatrix[0] * gl_MultiTexCoord2;
}

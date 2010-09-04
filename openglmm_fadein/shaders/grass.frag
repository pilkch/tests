
uniform sampler2D texUnit0; // Base texture

const float tolerance = 0.2;

void main( void )
{
  vec4 colour = texture2D(texUnit0, gl_TexCoord[0].st);
  if (colour.a < tolerance) discard;

  gl_FragColor = colour;
}

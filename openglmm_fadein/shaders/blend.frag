
uniform sampler2D texUnit0; // Texture0
uniform sampler2D texUnit1; // Texture1

uniform float fBlend0;
uniform float fBlend1;

void main(void)
{
  vec3 texture0 = texture2D(texUnit0, gl_TexCoord[0].st).rgb;
  vec3 texture1 = texture2D(texUnit1, gl_TexCoord[0].st).rgb;

  gl_FragColor = vec4((fBlend0 * texture0) + (fBlend1 * texture1), 1.0);
}


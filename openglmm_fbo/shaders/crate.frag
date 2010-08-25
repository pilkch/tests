
uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Detail texture

void main(void)
{
  vec4 diffuse = texture2D(texUnit0, gl_TexCoord[0].st);
  vec4 detail = texture2D(texUnit1, gl_TexCoord[1].st);

  gl_FragColor = vec4((diffuse * detail).rgb, 1.0);
}

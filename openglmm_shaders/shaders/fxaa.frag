#version 330

// FXAA
// http://horde3d.org/wiki/index.php5?title=Shading_Technique_-_FXAA

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColor;


#define FXAA_REDUCE_MIN   (1.0 / 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     8.0
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)


void main()
{
  vec4 posPos = vec4(vertOutTexCoord.xy, vertOutTexCoord.xy -((0.5 + FXAA_SUBPIX_SHIFT)));

  vec3 rgbNW = texture(texUnit0, posPos.zw).xyz;
  vec3 rgbNE = texture(texUnit0, posPos.zw + vec2(1.0,0.0)).xyz;
  vec3 rgbSW = texture(texUnit0, posPos.zw + vec2(0.0,1.0)).xyz;
  vec3 rgbSE = texture(texUnit0, posPos.zw + vec2(1.0,1.0)).xyz;
  vec3 rgbM  = texture(texUnit0, posPos.xy).xyz;
  
  const vec3 luma = vec3(0.299, 0.587, 0.114);

  float lumaNW = dot(rgbNW, luma);
  float lumaNE = dot(rgbNE, luma);
  float lumaSW = dot(rgbSW, luma);
  float lumaSE = dot(rgbSE, luma);
  float lumaM  = dot(rgbM,  luma);

  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

  vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

  float dirReduce = max(
	  (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
	  FXAA_REDUCE_MIN);
  float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
  dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
		  max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
		  dir * rcpDirMin));

  vec4 rgbA = (1.0/2.0) * (
	  texture(texUnit0, posPos.xy + dir * (1.0/3.0 - 0.5)) +
	  texture(texUnit0, posPos.xy + dir * (2.0/3.0 - 0.5)));
  vec4 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
	  texture(texUnit0, posPos.xy + dir * (0.0/3.0 - 0.5)) +
	  texture(texUnit0, posPos.xy + dir * (3.0/3.0 - 0.5)));
  float lumaB = dot(rgbB, vec4(luma, 0.0));

  if((lumaB < lumaMin) || (lumaB > lumaMax)) fragmentColor = rgbA;
  else fragmentColor = rgbB;
}

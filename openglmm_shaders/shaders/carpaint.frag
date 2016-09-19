#version 330 core

// Stolen from:
// http://blog.2pha.com/demos/threejs/shaders/9700_car_shader/car_shader_5.html

uniform sampler2D texUnit0; // Normal map
uniform sampler2D texUnit1; // Micro flake normal map
uniform samplerCube texUnit2; // Cubemap texture

uniform vec3 paintColor1 = vec3(0.4, 0.0, 0.35);
uniform vec3 paintColor2 = vec3(0.6, 0.0, 0.0);
uniform vec3 paintColor3 = vec3(1.0, 1.0, 0.0);
uniform vec3 flakeColor = vec3(1.0, 1.0, 0.0);

uniform float normalPerturbation = 1.0;
uniform float microflakePerturbationA = 0.1;
uniform float microflakePerturbation = 0.48;

uniform float glossLevel = 1.0;
uniform float brightnessFactor = 1.0;

uniform float flakeScale = -10.0;
uniform float normalScale = 1.0;

smooth in vec2 vertOutTexCoord0;
smooth in vec3 worldNormal;
smooth in vec3 mvPosition;
smooth in vec3 cameraToVertex;
smooth in vec3 lightToVertex;

out vec4 fragmentColour;

// This function taken directly from the three.js phong fragment shader.
// http://hacksoflife.blogspot.ch/2009/11/per-pixel-tangent-space-normal-mapping.html
vec3 perturbNormal2Arb( vec3 eye_pos, vec3 surf_norm )
{
  vec3 q0 = dFdx( eye_pos );
  vec3 q1 = dFdy( eye_pos );
  vec2 st0 = dFdx( vertOutTexCoord0 );
  vec2 st1 = dFdy( vertOutTexCoord0 );

  vec3 S = normalize( q0 * st1.t - q1 * st0.t );
  vec3 T = normalize( -q0 * st1.s + q1 * st0.s );
  vec3 N = normalize( surf_norm );

  vec3 mapN = texture( texUnit0, vertOutTexCoord0 ).xyz * 2.0 - 1.0;
  mapN.xy = normalScale * mapN.xy;
  mat3 tsn = mat3( S, T, N );
  return normalize( tsn * mapN );
}

vec3 perturbSparkleNormal2Arb( vec3 eye_pos, vec3 surf_norm )
{
    vec3 q0 = dFdx( eye_pos.xyz );
    vec3 q1 = dFdy( eye_pos.xyz );
    vec2 st0 = dFdx( vertOutTexCoord0 );
    vec2 st1 = dFdy( vertOutTexCoord0 );

    vec3 S = normalize( q0 * st1.t - q1 * st0.t );
    vec3 T = normalize( -q0 * st1.s + q1 * st0.s );
    vec3 N = normalize( surf_norm );

    vec2 flakeUv = flakeScale * vertOutTexCoord0;
    vec3 mapN = texture( texUnit1, flakeUv ).xyz * 2.0 - 1.0;
    mapN.xy = 1.0 * mapN.xy;
    mat3 tsn = mat3( S, T, N );
    return normalize( tsn * mapN );
}

vec3 CalculateMultiTonePaint()
{
  // Flakes
  vec3 vFlakesNormal = perturbSparkleNormal2Arb(mvPosition, worldNormal);
  vec3 vNp1 = microflakePerturbationA * vFlakesNormal + normalPerturbation * worldNormal;
  vec3 vNp2 = microflakePerturbation * ( vFlakesNormal + worldNormal ) ;

  float  fFresnel1 = clamp(dot( -cameraToVertex, vNp1 ), 0.0, 1.0);
  float  fFresnel2 = clamp(dot( -lightToVertex, vNp2 ), 0.0, 1.0);

  float fFresnel1Sq = fFresnel1 * fFresnel1;
  // Original
  //return fFresnel1   * paintColor1 +
  //       fFresnel1Sq * paintColor2 +
  //       fFresnel1Sq * fFresnel1Sq * paintColor3 +
  //       pow( fFresnel2, 16.0 ) * flakeColor;

  return (
    0.5 * (fFresnel1   * paintColor1) +
    6.0 * (fFresnel1Sq * paintColor2) +
    1.0 * (fFresnel1Sq * fFresnel1Sq * paintColor3) +
    0.5 * (fFresnel2 * flakeColor)
  );
}

vec3 CalculateReflection()
{
  //vec3 I = normalize(position - cameraPos);
  //vec3 R = reflect(I, normalize(normal));
  //return texture(texUnit2, R).rgb;

  vec3 normal = perturbNormal2Arb( mvPosition, worldNormal );
  float fFresnel = dot( normalize( -cameraToVertex ), normal );
  vec3 reflection = 2.0 * worldNormal * fFresnel - normalize(-cameraToVertex);
  float fEnvContribution = 1.0 - 0.5 * fFresnel;
  return fEnvContribution * brightnessFactor * texture( texUnit2, vec3( -reflection.x, reflection.yz ), glossLevel ).rgb;
}

void main()
{
  // Reflection
  vec3 reflectionColour = CalculateReflection();

  // Calculate multitone paint
  vec3 paintColour = CalculateMultiTonePaint();

  fragmentColour = vec4(4.0 * paintColour + 1.0 * reflectionColour, 1.0);
}

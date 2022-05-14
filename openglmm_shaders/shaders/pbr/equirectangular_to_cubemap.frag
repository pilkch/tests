#version 330 core

//uniform sampler2D equirectangularMap;
uniform sampler2DRect equirectangularMap;

smooth in vec3 WorldPos;
smooth in vec2 TexCoord;

out vec4 fragmentColour;

// Convert from cartesian coordinates to polar angles to uvs
// https://stackoverflow.com/a/48534536/1074390
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    // Convert from cartesian coordinates to polar angles
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));

    // Convert to uv coordinates
    uv *= invAtan;

    // Convert texture coordinates from -0.5..0.5 to 0..1
    uv += 0.5;

    return uv;
}

float getTheta(float x, float y)
{
  float rtn = 0;
  if (y < 0) {
    rtn = atan2(y, x) * -1;
  } else {
    rtn = PI + (PI - atan2(y, x));
  }
  return rtn;
}
vec2 GetPolarCoordinatesFromPoint(vec3 v)
{
    float rho = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    float normTheta = getTheta(v.x, v.z) / (2 * PI); // Divide by (2*PI) normalise theta
    float normPhi = (PI - acos(v.y / rho)) / PI; // Divide by PI normalise phi

    return vec2(normTheta, normPhi);
}

void main()
{
    // World position -1.0 to 1.0 is passed in, so we need to scale this by the texture size
    vec2 textureDimensions = textureSize(equirectangularMap);

    vec2 polarCoordinates = GetPolarCoordinatesFromPoint(normalize(WorldPos));
    vec2 uv = vec2(1.0, 1.0) * textureDimensions * polarCoordinates;

    vec3 color = texture(equirectangularMap, uv).rgb;

    fragmentColour = vec4(color, 1.0);
}

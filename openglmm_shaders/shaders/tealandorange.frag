#version 330

// http://theabyssgazes.blogspot.com/2010/03/teal-and-orange-hollywood-please-stop.html
// http://theabyssgazes.blogspot.com.au/2010/03/teal-and-orange-part-2.html

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColor;

float min3(float a, float b, float c)
{
  return ((a < b) ? min(a, c) : min(b, c));
}

float max3(float a, float b, float c)
{
  return ((a > b) ? max(a, c) : max(b, c));
}

// Convert RGB to HSL where each component ranges from 0 to 1
vec3 RGBtoHSL(vec3 colourRGB)
{
  float minc  = min3(colourRGB.r, colourRGB.g, colourRGB.b);
  float maxc  = max3(colourRGB.r, colourRGB.g, colourRGB.b);
  float delta = maxc - minc;

  float lum = (minc + maxc) * 0.5;
  float sat = 0.0;
  float hue = 0.0;

  if (lum > 0.0 && lum < 1.0) {
    float mul = (lum < 0.5) ? lum : (1.0 - lum);
    sat = delta / (mul * 2.0);
  }

  vec3 masks = vec3(
    ((maxc == colourRGB.r) && (maxc != colourRGB.g)) ? 1.0 : 0.0,
    ((maxc == colourRGB.g) && (maxc != colourRGB.b)) ? 1.0 : 0.0,
    ((maxc == colourRGB.b) && (maxc != colourRGB.r)) ? 1.0 : 0.0
  );

  vec3 adds = vec3(
    ((colourRGB.g - colourRGB.b) / delta),
    2.0 + ((colourRGB.b - colourRGB.r) / delta),
    4.0 + ((colourRGB.r - colourRGB.g) / delta)
  );

  float deltaGtz = (delta > 0.0) ? 1.0 : 0.0;

  hue += dot(adds, masks);
  hue *= deltaGtz;
  hue /= 6.0;

  if (hue < 0.0) hue += 1.0;

  return vec3(hue, sat, lum);
}

float GetHueOfColour(vec3 colour)
{
  vec3 colourHSL = RGBtoHSL(colour.rgb);

  return colourHSL.r;
}

void main()
{
  vec4 colour = texture(texUnit0, vertOutTexCoord);

  float fHue = GetHueOfColour(colour.rgb);

  // Teal
  const vec3 tealRGB = vec3(0, 1.0, 1.0);
  float fTealHue = GetHueOfColour(tealRGB);
  float fTealDistance = abs(fTealHue - fHue);

  // Orange
  const vec3 orangeRGB = vec3(1.0, 0.5, 0.0);
  float fOrangeHue = GetHueOfColour(orangeRGB);
  float fOrangeDistance = abs(fOrangeHue - fHue);

  // Make the colour either more teal or more orange depending on which this colour is closer to
  if (fTealDistance < fOrangeDistance) colour.rgb = mix(colour.rgb, vec3(0, 1.0, 1.0), 0.25);
  else colour.rgb = mix(colour.rgb, vec3(1.0, 0.5, 0.0), 0.25);

  fragmentColor = colour;
}

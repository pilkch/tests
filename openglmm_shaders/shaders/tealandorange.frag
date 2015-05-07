#version 330

// http://theabyssgazes.blogspot.com/2010/03/teal-and-orange-hollywood-please-stop.html
// http://theabyssgazes.blogspot.com.au/2010/03/teal-and-orange-part-2.html

uniform sampler2DRect texUnit0; // Diffuse texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColor;

#include <math.header>

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

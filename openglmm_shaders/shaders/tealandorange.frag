#version 330 core

// http://theabyssgazes.blogspot.com/2010/03/teal-and-orange-hollywood-please-stop.html
// http://theabyssgazes.blogspot.com.au/2010/03/teal-and-orange-part-2.html

#include <colour.header>

float GetHueOfColour(vec3 colour)
{
  vec3 colourHSL = RGBtoHSL(colour.rgb);

  return colourHSL.r;
}

vec4 ApplyFilter(vec4 inputColour)
{
  float fHue = GetHueOfColour(inputColour.rgb);

  // Teal
  const vec3 tealRGB = vec3(0, 1.0, 1.0);
  float fTealHue = GetHueOfColour(tealRGB);
  float fTealDistance = abs(fTealHue - fHue);

  // Orange
  const vec3 orangeRGB = vec3(1.0, 0.5, 0.0);
  float fOrangeHue = GetHueOfColour(orangeRGB);
  float fOrangeDistance = abs(fOrangeHue - fHue);

  // Make the colour either more teal or more orange depending on which this colour is closer to
  if (fTealDistance < fOrangeDistance) inputColour.rgb = mix(inputColour.rgb, vec3(0, 1.0, 1.0), 0.25);
  else inputColour.rgb = mix(inputColour.rgb, vec3(1.0, 0.5, 0.0), 0.25);

  return inputColour;
}

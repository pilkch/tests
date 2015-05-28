#version 330

#include <colour.header>

// http://www.daltonize.org/2010/05/lms-daltonization-algorithm.html
// https://gist.github.com/jcdickinson/580b7fb5cc145cee8740

#define COLORBLIND_MODE 3

#if (COLORBLIND_MODE == 3)
// Tritanope - blues are greatly reduced (0.003% population)
// http://mudcu.be/labs/index.php?dir=Color-Vision/Javascript/
const mat3 matTritanopia = mat3(
  0.972, 0.022, -0.063,
  0.112, 0.818, 0.881,
  -0.084, 0.160, 0.182
);
#endif

// Daltonize (source http://www.daltonize.org/search/label/Daltonize)
// Modified to simulate color blindness
vec4 ApplyFilter(vec4 inputColour)
{
  #if (COLORBLIND_MODE == 1) || (COLORBLIND_MODE == 2)
  // Get an LMS colour
  vec3 lms = RGBtoLMS(inputColour.rgb);
  float L = lms.x;
  float M = lms.y;
  float S = lms.z;

  // Simulate color blindness
  #if (COLORBLIND_MODE == 1)
  // Protanope - reds are greatly reduced (1% men)
  float l = 0.0f * L + 2.02344f * M + -2.52581f * S;
  float m = 0.0f * L + 1.0f * M + 0.0f * S;
  float s = 0.0f * L + 0.0f * M + 1.0f * S;
  #else
  // Deuteranope - greens are greatly reduced (1% men)
  float l = 1.0f * L + 0.0f * M + 0.0f * S;
  float m = 0.494207f * L + 0.0f * M + 1.24827f * S;
  float s = 0.0f * L + 0.0f * M + 1.0f * S;
  #endif

  // LMS to RGB matrix conversion
  vec3 colourRGB = LMStoRGB(l, m, s);

  return vec4(colourRGB, inputColour.a);
  #else
  // Tritanope - blues are greatly reduced (0.003% population)
  // This didn't seem to work in the above code, so we just multiply each colour by a matrix instead
  //float l = 1.0f * L + 0.0f * M + 0.0f * S;
  //float m = 0.0f * L + 1.0f * M + 0.0f * S;
  //float s = -0.395913f * L + 0.801109f * M + 0.0f * S;

  return vec4(matTritanopia * inputColour.rgb, inputColour.a);
  #endif
}

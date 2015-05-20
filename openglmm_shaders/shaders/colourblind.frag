#version 330

#include <colour.header>

// http://www.daltonize.org/2010/05/lms-daltonization-algorithm.html
// https://gist.github.com/jcdickinson/580b7fb5cc145cee8740

// Daltonize (source http://www.daltonize.org/search/label/Daltonize)
// Modified to simulate color blindness
vec4 ApplyFilter(vec4 inputColour)
{
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
#elif (COLORBLIND_MODE == 2)
  // Deuteranope - greens are greatly reduced (1% men)
	float l = 1.0f * L + 0.0f * M + 0.0f * S;
	float m = 0.494207f * L + 0.0f * M + 1.24827f * S;
	float s = 0.0f * L + 0.0f * M + 1.0f * S;
#else
  // Tritanope - blues are greatly reduced (0.003% population)
	float l = 1.0f * L + 0.0f * M + 0.0f * S;
	float m = 0.0f * L + 1.0f * M + 0.0f * S;
	float s = -0.395913f * L + 0.801109f * M + 0.0f * S;
#endif

	// LMS to RGB matrix conversion
	vec3 colourRGB = LMStoRGB(l, m, s);

	return vec4(colourRGB, inputColour.a);
}

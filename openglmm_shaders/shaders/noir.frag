#version 330 core

vec4 ApplyFilter(vec4 inputColour)
{
  if ((inputColour.r > 0.4) && (inputColour.b < 0.05) && (inputColour.g < 0.05)) {
    // Red colour is dominant, emphasise it
    inputColour.rgb = mix(inputColour.rgb, vec3(1.0, 0.0, 0.0), 0.9);
  } else if ((inputColour.r > 0.4) && (inputColour.b < 0.1) && (inputColour.g < 0.1)) {
    // Red colour is mostly dominant, emphasise it
    inputColour.rgb = mix(inputColour.rgb, vec3(1.0, 0.0, 0.0), 0.7);
  } else {
    // Greyscale
    float fSin = max(inputColour.r, max(inputColour.g, inputColour.b)) / 2 + min(inputColour.r, min(inputColour.g, inputColour.b)) / 2;

    if ((fSin < 0.1) || (fSin > 0.95)) {
      inputColour.rgb = vec3(0.0);
    } else {
      inputColour.rgb = vec3(1.0);
    }
  }

  return inputColour;
}

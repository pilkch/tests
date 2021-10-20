#version 330 core

vec3 desaturate(vec3 color, float amount)
{
  vec3 greyscale = vec3(dot(vec3(0.2126, 0.7152, 0.0722), color));
  return vec3(mix(color, greyscale, amount));
}

vec4 ApplyFilter(vec4 inputColour)
{
  inputColour.rgb = desaturate(inputColour.rgb, 0.1);

  inputColour.g += 0.2;

  return inputColour;
}

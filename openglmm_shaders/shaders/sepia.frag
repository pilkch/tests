#version 330 core

// http://stackoverflow.com/questions/5830139/where-can-i-find-sample-opengl-es-2-0-shaders-that-perform-image-processing-task/9402041#9402041

const lowp mat4 colorMatrix = mat4(
  0.3588, 0.7044, 0.1368, 0,
  0.2990, 0.5870, 0.1140, 0,
  0.2392, 0.4696, 0.0912, 0,
  0, 0, 0, 0
);

lowp float fIntensity = 1.0;

vec4 ApplyFilter(vec4 inputColour)
{
  vec4 sepiaColour = inputColour * colorMatrix;

  return (fIntensity * sepiaColour) + ((1.0 - fIntensity) * inputColour);
}

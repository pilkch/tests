#version 330 core

// Ouput
layout(location = 0) out float fFragmentdepth;

void main()
{
  fFragmentdepth = gl_FragCoord.z;
}

#version 330

uniform sampler2DRect texUnit0; // Scene texture

smooth in vec2 vertOutTexCoord;

out vec4 fragmentColour;

void main()
{
  const uint samples = uint(16); // We are downsampling 16 times

  // Downsampling-offsets in pixel. Note that there is no "0.0", cause this point lies on the middle of four texels
  float samplerOffset[samples];
  samplerOffset[0] = 1.0f;
  samplerOffset[1] = 2.0f;
  samplerOffset[2] = 3.0f;
  samplerOffset[3] = 4.0f;
  samplerOffset[4] = 3.0f;
  samplerOffset[5] = 2.0f;
  samplerOffset[6] = 2.0f;
  samplerOffset[7] = 1.0f;
  samplerOffset[8] = -1.0f;
  samplerOffset[9] = -2.0f;
  samplerOffset[10] = -3.0f;
  samplerOffset[11] = -4.0f;
  samplerOffset[12] = -5.0f;
  samplerOffset[13] = -6.0f;
  samplerOffset[14] = -7.0f;
  samplerOffset[15] = -8.0f;	
  
  vec3 result = vec3(0.0);
  for (uint i = uint(0); i < samples; i++) {
    // Translate the offset(in pixels) to normalized texture coordinates, by multiplying offset[i] with the size of one pixel in the target texture
    vec2 texOffset = vec2(samplerOffset[i], 0.0f);

    // Sample the pixel and add its color to the result, so that we can compute the average
    result += texture(texUnit0, vertOutTexCoord + texOffset).rgb;
  }
  
  // Get the average by dividing the result by the number of downsample steps
  vec3 colour = result / float(samples);


  fragmentColour = vec4(colour, 1.0);
}

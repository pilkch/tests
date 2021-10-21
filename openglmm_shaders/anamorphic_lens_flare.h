#ifndef ANAMORPHIC_LENS_FLARE_H
#define ANAMORPHIC_LENS_FLARE_H

// Breathe headers
#include <breathe/render/model/cFileFormatOBJ.h>
#include <breathe/render/model/cStatic.h>

// libopenglmm headers
#include <libopenglmm/cContext.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cTexture.h>

class cApplication;

// ** cAnamorphicLensFlare
//
// Just a proof of concept, it should not be used in a real application as this method is very expensive, 6 passes plus the output to the final buffer
// The other way to create an anamorphic lens is to just create a stretched billboard at each light point in the scene, as is commonly done for normal lens flare light billboards
// It is basically just a horizontal blur of the bright pixels, over 6 passes we blur to smaller and smaller buffers to "stretch" the bright pixels left and right across the screen

class cAnamorphicLensFlare
{
public:
  void Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(opengl::cContext& context);

  void Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& inputBrightPixels, opengl::cTextureFrameBufferObject& output);

private:
  void RenderInputBrightPixelsBlurred(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output);
  void RenderBlurredToWithBlurTingeToOutput(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output);

  opengl::cShader shaderHorizontalBlur;
  opengl::cTextureFrameBufferObject brightPixelsStretchedHorizontally0;
  opengl::cTextureFrameBufferObject brightPixelsStretchedHorizontally1;
  opengl::cTextureFrameBufferObject brightPixelsStretchedHorizontally2;
  opengl::cTextureFrameBufferObject brightPixelsStretchedHorizontally3;
  opengl::cTextureFrameBufferObject brightPixelsStretchedHorizontally4;
  opengl::cTextureFrameBufferObject brightPixelsStretchedHorizontally5;
  opengl::cShader shaderAnamorphicLensBlueTinge;
};

#endif // ANAMORPHIC_LENS_FLARE_H

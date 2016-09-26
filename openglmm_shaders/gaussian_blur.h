#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

// Spitfire headers
#include <spitfire/spitfire.h>

// libopenglmm headers
#include <libopenglmm/cContext.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>

// Application headers
#include "util.h"

class cApplication;

class cGaussianBlur
{
public:
  void Init(cApplication& application, opengl::cContext& context, size_t fboWidth, size_t fboHeight);
  void Destroy(opengl::cContext& context);

  void Resize(opengl::cContext& context);

  // Apply a Gaussian blur to the 0th color attachment of input fbo. The result is written to output fbo, which may be the same as input
  void Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& temp, opengl::cTextureFrameBufferObject& output, unsigned int uiRadiusPixels);

private:
  opengl::cShader shaderGaussBlur;

  opengl::cStaticVertexBufferObject vbo;
};

#endif // GAUSSIAN_BLUR_H

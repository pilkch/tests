#ifndef LENSFLAREDIRT_H
#define LENSFLAREDIRT_H

#include <cassert>
#include <cmath>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>
#include <list>

// OpenGL headers
#include <GL/GLee.h>
#include <GL/glu.h>

// SDL headers
#include <SDL2/SDL_image.h>

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/util/timer.h>
#include <spitfire/util/thread.h>

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>

#include <spitfire/storage/file.h>
#include <spitfire/storage/filesystem.h>
#include <spitfire/util/log.h>

// libopenglmm headers
#include <libopenglmm/libopenglmm.h>
#include <libopenglmm/cContext.h>
#include <libopenglmm/cFont.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Application headers
#include "util.h"

class cApplication;

class cLensFlareDirt
{
public:
  cLensFlareDirt();

  void Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(cApplication& application, opengl::cContext& context);

  void Render(cApplication& application, float dt, opengl::cContext& context, opengl::cTextureFrameBufferObject& fboIn, opengl::cTextureFrameBufferObject& fboOut);

private:
  /*	Apply a Gaussian blur to the 0th color attachment of input fbo. The result
  is written to output fbo, which may be the same as in. */
  void gaussBlur(
    opengl::cTextureFrameBufferObject* in,
    opengl::cTextureFrameBufferObject* temp,
    opengl::cTextureFrameBufferObject* out, // can be the same as in
    int radius // blur kernel radius in texels
    );

  /*	Renders to every mip level of the 0th color attachment of input fbo.
  Assumes that shader is already in use. */
  void renderToMipmap(
    opengl::cTextureFrameBufferObject* fbo,
    opengl::cShader* pShader,
    int bindLocation = 0 // used as GL_TEXTURE0 + bindLocation
    );

  /*	Resize temp textures; call when changing tempBufferSize_. */
  bool resizeTempBuffers();

  opengl::cShader* shaderPostProcess_;
  float exposure_; // exposure override

  opengl::cShader* shaderGaussBlur_;

  opengl::cShader* shaderScaleBias_;
  float bloomScale_, bloomBias_;
  float bloomBlurRadius_;

  //	lens flare:
  opengl::cTexture* texLensColor_; // radial feature colour
  opengl::cTexture* texLensDirt_;
  opengl::cTexture* texLensStar_; // dirt/diffraction starburst
  opengl::cShader* shaderLensflare_; // feature generation
  float flareSamples_, flareDispersal_, flareHaloWidth_, flareDistortion_;
  float flareScale_, flareBias_;
  float flareBlurRadius_;
  bool flareOnly_;

  //	render textures/framebuffers:
  opengl::cTextureFrameBufferObject* fboHdr_;
  int tempBufferSize_; // divides render size
  float setTempBufferSize_; // slider controlled; sets tempBufferSize_
  opengl::cTextureFrameBufferObject* fboTempA_;
  opengl::cTextureFrameBufferObject* fboTempB_;
  opengl::cTextureFrameBufferObject* fboTempC_;

  //	auto exposure:
  opengl::cTextureFrameBufferObject* fboLuma_;
  opengl::cTextureFrameBufferObject* fboAdaptLuma_[2];
  opengl::cShader* shaderLuminance_;
  opengl::cShader* shaderAvgMinMax_;
  opengl::cShader* shaderLuminanceAdapt_;
  int currentAdaptLuma_; // swap between fboAdaptLuma_ per frame
  float adaptionRate_;
};

#endif // LENSFLAREDIRT_H

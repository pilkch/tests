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

  void Init(cApplication& application, opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(opengl::cContext& context);

  void Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& fboIn, opengl::cTextureFrameBufferObject& fboOut, float fExposure, bool bDebugShowFlareOnly);

  opengl::cTexture& GetTextureLensColor() { return *texLensColor_; }
  opengl::cTexture& GetTextureLensDirt() { return *texLensDirt_; }
  opengl::cTexture& GetTextureLensStar() { return *texLensStar_; }

  opengl::cTextureFrameBufferObject& GetTempA() { return *fboTempA_; }
  opengl::cTextureFrameBufferObject& GetTempB() { return *fboTempB_; }
  opengl::cTextureFrameBufferObject& GetTempC() { return *fboTempC_; }

private:
  /*	Apply a Gaussian blur to the 0th color attachment of input fbo. The result
  is written to output fbo, which may be the same as in. */
  void gaussBlur(
    cApplication& application,
    opengl::cContext& context,
    opengl::cTextureFrameBufferObject& in,
    opengl::cTextureFrameBufferObject& temp,
    opengl::cTextureFrameBufferObject& out, // can be the same as in
    int radius // blur kernel radius in texels
    );

  void CreateTempBuffers(cApplication& application, opengl::cContext& context);
  void DestroyTempBuffers(opengl::cContext& context);

  opengl::cShader* shaderPostProcess_;
  opengl::cShader* shaderGaussBlur_;
  opengl::cShader* shaderScaleBias_;
  opengl::cStaticVertexBufferObject vboScaleBias;

  // Lens flare
  opengl::cTexture* texLensColor_; // radial feature colour
  opengl::cTexture* texLensDirt_;
  opengl::cTexture* texLensStar_; // dirt/diffraction starburst
  opengl::cShader* shaderLensflare_; // feature generation
  float flareScale_;
  float flareBias_;
  int flareSamples_;
  float flareDispersal_;
  float flareHaloWidth_;
  float flareDistortion_;
  float flareBlurRadius_;
  opengl::cStaticVertexBufferObject vboLensFlare;

  // Render textures/framebuffers
  int tempBufferSize_; // divides render size
  float setTempBufferSize_; // slider controlled; sets tempBufferSize_
  opengl::cTextureFrameBufferObject* fboTempA_;
  opengl::cTextureFrameBufferObject* fboTempB_;
  opengl::cTextureFrameBufferObject* fboTempC_;
};

#endif // LENSFLAREDIRT_H

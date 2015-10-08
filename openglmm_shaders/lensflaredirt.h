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

class cShadowMapping
{
public:
  cShadowMapping();

  void Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(cApplication& application, opengl::cContext& context);

  void Render(cApplication& application, opengl::cContext& context, float dt);

private:
  /*	Apply a Gaussian blur to the 0th color attachment of input fbo. The result
  is written to output fbo, which may be the same as in. */
  void gaussBlur(
    frm::Framebuffer *in,
    frm::Framebuffer *aux,
    frm::Framebuffer *out, // can be the same as in
    int radius // blur kernel radius in texels
    );

  /*	Renders to every mip level of the 0th color attachment of input fbo.
  Assumes that shader is already in use. */
  void renderToMipmap(
    frm::Framebuffer *fbo,
    frm::Shader *shader,
    int bindLocation = 0 // used as GL_TEXTURE0 + bindLocation
    );

  /*	Resize aux textures; call when changing auxBufferSize_. */
  bool resizeAuxBuffers();

  frm::Shader *shaderPostProcess_;
  float exposure_; // exposure override

  frm::Shader *shaderGaussBlur_;

  frm::Shader *shaderScaleBias_;
  float bloomScale_, bloomBias_;
  float bloomBlurRadius_;

  //	lens flare:
  frm::Texture1d *texLensColor_; // radial feature colour
  frm::Texture2d *texLensDirt_, *texLensStar_; // dirt/diffraction starburst
  frm::Shader *shaderLensflare_; // feature generation
  float flareSamples_, flareDispersal_, flareHaloWidth_, flareDistortion_;
  float flareScale_, flareBias_;
  float flareBlurRadius_;
  bool flareOnly_;

  //	render textures/framebuffers:
  frm::Framebuffer *fboHdr_;
  frm::Texture2d *texHdr_, *texDepth_;
  int auxBufferSize_; // divides render size
  float setAuxBufferSize_; // slider controlled; sets auxBufferSize_
  frm::Framebuffer *fboAuxA_, *fboAuxB_, *fboAuxC_;
  frm::Texture2d *texAuxA_, *texAuxB_, *texAuxC_;

  //	auto exposure:
  frm::Framebuffer *fboLuma_, *fboAdaptLuma_[2];
  frm::Texture2d *texLuma_, *texAdaptLuma_[2];
  frm::Shader *shaderLuminance_, *shaderAvgMinMax_, *shaderLuminanceAdapt_;
  int currentAdaptLuma_; // swap between fboAdaptLuma_ per frame
  float adaptionRate_;
};

#endif // LENSFLAREDIRT_H

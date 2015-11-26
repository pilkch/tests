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
#include "lensflaredirt.h"
#include "main.h"

// ** cLensFlareDirt

cLensFlareDirt::cLensFlareDirt() :
  flareScale_(1.0f),
  flareBias_(-0.04f),
  flareSamples_(14.0f),
  flareDispersal_(0.3f),
  flareHaloWidth_(0.6f),
  flareDistortion_(10.5f),
  flareBlurRadius_(24.0f),
  tempBufferSize_(12),
  setTempBufferSize_(12.0f)
{
}

void cLensFlareDirt::Init(cApplication& application, opengl::cContext& context)
{
  //------------------------------------------------------------------------------
  // TEXTURES
  //------------------------------------------------------------------------------

  texLensDirt_ = context.CreateTexture(TEXT("textures/lensdirt.png"));
  ASSERT(texLensDirt_ != nullptr);
  texLensDirt_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  texLensDirt_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  texLensStar_ = context.CreateTexture(TEXT("textures/lensstar.png"));
  ASSERT(texLensStar_ != nullptr);
  texLensStar_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  texLensStar_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  texLensColor_ = context.CreateTexture(TEXT("textures/lenscolor.png"));
  ASSERT(texLensColor_ != nullptr);
  texLensColor_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  texLensColor_->SetWrap(opengl::TEXTURE_WRAP::REPEAT);



  //------------------------------------------------------------------------------
  // SHADERS
  //------------------------------------------------------------------------------
  shaderScaleBias_ = context.CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/lensflaredirt/scalebias.frag"));
  ASSERT(shaderScaleBias_ != nullptr);
  shaderGaussBlur_ = context.CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/lensflaredirt/gauss1d.frag"));
  ASSERT(shaderGaussBlur_ != nullptr);
  shaderLensflare_ = context.CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/lensflaredirt/lensflare.frag"));
  ASSERT(shaderLensflare_ != nullptr);
  shaderPostProcess_ = context.CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/lensflaredirt/postprocess.frag"));
  ASSERT(shaderPostProcess_ != nullptr);


  CreateTempBuffers(application, context);
}

void cLensFlareDirt::Destroy(opengl::cContext& context)
{
  DestroyTempBuffers(context);


  if (shaderPostProcess_ != nullptr) {
    context.DestroyShader(shaderPostProcess_);
    shaderPostProcess_ = nullptr;
  }
  if (shaderGaussBlur_ != nullptr) {
    context.DestroyShader(shaderGaussBlur_);
    shaderGaussBlur_ = nullptr;
  }
  if (shaderScaleBias_ != nullptr) {
    context.DestroyShader(shaderScaleBias_);
    shaderScaleBias_ = nullptr;
  }
  if (shaderLensflare_ != nullptr) {
    context.DestroyShader(shaderLensflare_);
    shaderLensflare_ = nullptr;
  }


  if (texLensDirt_ != nullptr) {
    context.DestroyTexture(texLensDirt_);
    texLensDirt_ = nullptr;
  }
  if (texLensStar_ != nullptr) {
    context.DestroyTexture(texLensStar_);
    texLensStar_ = nullptr;
  }
  if (texLensColor_ != nullptr) {
    context.DestroyTexture(texLensColor_);
    texLensColor_ = nullptr;
  }
}

void cLensFlareDirt::CreateTempBuffers(cApplication& application, opengl::cContext& context)
{
  const size_t width = context.GetWidth();
  const size_t height = context.GetHeight();

  const size_t fboWidth = width / tempBufferSize_;
  const size_t fboHeight = height / tempBufferSize_;

  // Frame buffer objects
  fboTempA_ = context.CreateTextureFrameBufferObjectWithDepth(fboWidth, fboHeight);
  ASSERT(fboTempA_ != nullptr);
  fboTempA_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempA_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  fboTempB_ = context.CreateTextureFrameBufferObjectWithDepth(fboWidth, fboHeight);
  ASSERT(fboTempB_ != nullptr);
  fboTempB_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempB_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  fboTempC_ = context.CreateTextureFrameBufferObjectWithDepth(fboWidth, fboHeight);
  ASSERT(fboTempC_ != nullptr);
  fboTempC_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempC_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  // VBOs
  application.CreateScreenRectVBO(vboScaleBias, 1.0f, 1.0f, width, height);
  application.CreateScreenRectVBO(vboLensFlare, 1.0f, 1.0f, fboWidth, fboHeight);
}

void cLensFlareDirt::DestroyTempBuffers(opengl::cContext& context)
{
  // VBOs
  context.DestroyStaticVertexBufferObject(vboLensFlare);
  context.DestroyStaticVertexBufferObject(vboScaleBias);

  // Frame buffer objects
  if (fboTempA_ != nullptr) {
    context.DestroyTextureFrameBufferObject(fboTempA_);
    fboTempA_ = nullptr;
  }
  if (fboTempB_ != nullptr) {
    context.DestroyTextureFrameBufferObject(fboTempB_);
    fboTempB_ = nullptr;
  }
  if (fboTempC_ != nullptr) {
    context.DestroyTextureFrameBufferObject(fboTempC_);
    fboTempC_ = nullptr;
  }
}

void cLensFlareDirt::Resize(opengl::cContext& context)
{

}

void cLensFlareDirt::gaussBlur(
  cApplication& application,
  opengl::cContext& context,
  opengl::cTextureFrameBufferObject& in,
  opengl::cTextureFrameBufferObject& temp,
  opengl::cTextureFrameBufferObject& out,
  int radius
  )
{
  context.BindShader(*shaderGaussBlur_);
  context.SetShaderConstant("uBlurRadius", radius);

  // horizontal pass:
  context.BeginRenderToTexture(temp);
  context.SetShaderConstant("uBlurDirection", spitfire::math::cVec2(1.0f, 0.0f));
  context.BindTexture(0, in);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboLensFlare);
  context.UnBindTexture(0, in);
  context.EndRenderToTexture(temp);

  // vertical pass:
  context.BeginRenderToTexture(out);
  context.SetShaderConstant("uBlurDirection", spitfire::math::cVec2(0.0f, 1.0f));
  context.BindTexture(0, temp);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboLensFlare);
  context.UnBindTexture(0, temp);
  context.EndRenderToTexture(out);

  context.UnBindShader(*shaderGaussBlur_);
}

void cLensFlareDirt::Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& fboIn, opengl::cTextureFrameBufferObject& fboOut, bool bDebugShowFlareOnly)
{
  if ((int)setTempBufferSize_ != tempBufferSize_) {
    tempBufferSize_ = (int)setTempBufferSize_;

    DestroyTempBuffers(context);
    CreateTempBuffers(application, context);
  }

  //---------------------------------------------------------------------------
  //	GENERATE LENS FLARE
  //---------------------------------------------------------------------------

  //	downsample/threshold:
  context.BeginRenderToTexture(*fboTempA_);
  context.BindShader(*shaderScaleBias_);
  context.SetShaderConstant("uScale", spitfire::math::cVec3(flareScale_));
  context.SetShaderConstant("uBias", spitfire::math::cVec3(flareBias_));
  context.BindTexture(0, fboIn);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboScaleBias);
  context.UnBindTexture(0, fboIn);
  context.UnBindShader(*shaderScaleBias_);
  context.EndRenderToTexture(*fboTempA_);

  // feature generation:
  context.BeginRenderToTexture(*fboTempB_);
  context.BindShader(*shaderLensflare_);
  context.SetShaderConstant("uSamples", (int)flareSamples_);
  context.SetShaderConstant("uDispersal", flareDispersal_);
  context.SetShaderConstant("uHaloWidth", flareHaloWidth_);
  context.SetShaderConstant("uDistortion", flareDistortion_);
  context.BindTexture(0, *fboTempA_);
  context.BindTexture(1, *texLensColor_);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboLensFlare);
  context.UnBindTexture(1, *texLensColor_);
  context.UnBindTexture(0, *fboTempA_);
  context.UnBindShader(*shaderLensflare_);
  context.EndRenderToTexture(*fboTempB_);

  //	gaussian blur:
  gaussBlur(application, context, *fboTempB_, *fboTempA_, *fboTempC_, (int)flareBlurRadius_);


  // We can't upscale/blend as per bloom; we need to apply the lens flare after motion blur, etc. in the final post process


  //---------------------------------------------------------------------------
  //	FINAL POST PROCESS (TONEMAP, MBLUR, APPLY LENSFLARE, ETC.):
  //---------------------------------------------------------------------------

  //	transformation matrix for lens flare starburst:
  const spitfire::math::cMat4 matView = application.camera.CalculateViewMatrix();
  const spitfire::math::cVec3 camx = matView.GetColumn(0).GetXYZ();
  const spitfire::math::cVec3 camz = matView.GetColumn(1).GetXYZ();
  float camrot = 2.0f * (camx.DotProduct(spitfire::math::cVec3(0, 0, 1)) + camz.DotProduct(spitfire::math::cVec3(0, 1, 0)));

  spitfire::math::cMat4 matStarTranslation;
  matStarTranslation.SetTranslation(0.5f, 0.5f, 0.0f);

  spitfire::math::cMat4 matStarRotation;
  matStarRotation.SetRotationZ(camrot);

  const spitfire::math::cMat4 matStarTranslationRotation = matStarTranslation * matStarRotation;

  spitfire::math::cMat4 matStarUndoTranslation;
  matStarUndoTranslation.SetTranslation(-0.5f, -0.5f, 0.0f);

  const spitfire::math::cMat4 matStar = matStarTranslationRotation * matStarUndoTranslation;

  context.BeginRenderToTexture(fboOut);

  context.BindShader(*shaderPostProcess_);

  context.SetShaderConstant("matCameraLensStarBurst", matStar);
  context.SetShaderConstant("fSceneMix0To1", bDebugShowFlareOnly ? 0.0f : 1.0f);
  context.SetShaderConstant("fFlareMix0To1", 2.0f);

  context.BindTexture(0, fboIn);
  context.BindTexture(1, *fboTempC_);
  context.BindTexture(2, *texLensDirt_);
  context.BindTexture(3, *texLensStar_);

  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboScaleBias);

  context.UnBindTexture(3, *texLensStar_);
  context.UnBindTexture(2, *texLensDirt_);
  context.UnBindTexture(1, *fboTempC_);
  context.UnBindTexture(0, fboIn);

  context.UnBindShader(*shaderPostProcess_);

  context.EndRenderToTexture(fboOut);
}

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
  flareSamples_(8.0f),
  flareDispersal_(0.3f),
  flareHaloWidth_(0.6f),
  flareDistortion_(1.5f),
  flareScale_(0.05f),
  flareBias_(-3.5f),
  flareBlurRadius_(24.0f),
  tempBufferSize_(4),
  setTempBufferSize_(4.0f)
{
}

void cLensFlareDirt::Init(opengl::cContext& context)
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

  //... create 1D texture
  texLensColor_ = context.CreateTexture(TEXT("textures/lenscolor.png"));
  ASSERT(texLensColor_ != nullptr);
  texLensColor_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  texLensColor_->SetWrap(opengl::TEXTURE_WRAP::REPEAT);



  //------------------------------------------------------------------------------
  // SHADERS
  //------------------------------------------------------------------------------
  shaderPostProcess_ = context.CreateShader(TEXT("shaders/lensflaredirt/basic.vs.glsl"), TEXT("shaders/lensflaredirt/postprocess.fs.glsl"));
  ASSERT(shaderPostProcess_ != nullptr);
  shaderGaussBlur_ = context.CreateShader(TEXT("shaders/lensflaredirt/basic.vs.glsl"), TEXT("shaders/lensflaredirt/gauss1d.fs.glsl"));
  ASSERT(shaderGaussBlur_ != nullptr);
  shaderScaleBias_ = context.CreateShader(TEXT("shaders/lensflaredirt/basic.vs.glsl"), TEXT("shaders/lensflaredirt/scalebias.fs.glsl"));
  ASSERT(shaderScaleBias_ != nullptr);
  shaderLensflare_ = context.CreateShader(TEXT("shaders/lensflaredirt/basic.vs.glsl"), TEXT("shaders/lensflaredirt/lensflare.fs.glsl"));
  ASSERT(shaderLensflare_ != nullptr);


  CreateTempBuffers(context);
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

void cLensFlareDirt::CreateTempBuffers(opengl::cContext& context)
{
  const size_t width = context.GetWidth();
  const size_t height = context.GetHeight();

  //	AUX RENDER BUFFERS
  fboTempA_ = context.CreateTextureFrameBufferObject(width / tempBufferSize_, height / tempBufferSize_, opengl::PIXELFORMAT::R8G8B8A8);
  ASSERT(fboTempA_ != nullptr);
  fboTempA_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempA_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  fboTempB_ = context.CreateTextureFrameBufferObject(width / tempBufferSize_, height / tempBufferSize_, opengl::PIXELFORMAT::R8G8B8A8);
  ASSERT(fboTempB_ != nullptr);
  fboTempB_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempB_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  fboTempC_ = context.CreateTextureFrameBufferObject(width / tempBufferSize_, height / tempBufferSize_, opengl::PIXELFORMAT::R8G8B8A8);
  ASSERT(fboTempC_ != nullptr);
  fboTempC_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempC_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);
}

void cLensFlareDirt::DestroyTempBuffers(opengl::cContext& context)
{
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
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
  context.UnBindTexture(0, in);
  context.EndRenderToTexture(temp);

  // vertical pass:
  context.BeginRenderToTexture(out);
  context.SetShaderConstant("uBlurDirection", spitfire::math::cVec2(0.0f, 1.0f));
  context.BindTexture(0, temp);
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
  context.UnBindTexture(0, temp);
  context.EndRenderToTexture(out);

  context.UnBindShader(*shaderGaussBlur_);
}

void cLensFlareDirt::Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& fboIn, opengl::cTextureFrameBufferObject& fboOut)
{
  if ((int)setTempBufferSize_ != tempBufferSize_) {
    tempBufferSize_ = (int)setTempBufferSize_;

    DestroyTempBuffers(context);
    CreateTempBuffers(context);
  }

  //---------------------------------------------------------------------------
  //	GENERATE LENS FLARE
  //---------------------------------------------------------------------------

  //	downsample/threshold:
  context.BeginRenderToTexture(*fboTempA_);
  context.BindShader(*shaderScaleBias_);
  context.SetShaderConstant("uScale", spitfire::math::cVec4(flareScale_));
  context.SetShaderConstant("uBias", spitfire::math::cVec4(flareBias_));
  context.BindTexture(0, fboIn);
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
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
  // ... bind 1d?
  context.BindTexture(1, *texLensColor_);
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
  context.UnBindTexture(1, *texLensColor_);
  context.UnBindTexture(0, *fboTempA_);
  context.UnBindShader(*shaderLensflare_);
  context.EndRenderToTexture(*fboTempB_);

  //	gaussian blur:
  gaussBlur(application, context, *fboTempB_, *fboTempA_, *fboTempC_, (int)flareBlurRadius_);

  //	can't upscale/blend as per bloom; we need to apply the lens flare after
  //	motion blur, etc. in the final post process


  //---------------------------------------------------------------------------
  //	FINAL POST PROCESS (TONEMAP, MBLUR, APPLY LENSFLARE, ETC.):
  //---------------------------------------------------------------------------

  //	transformation matrix for lens flare starburst:
  const spitfire::math::cMat4 matView = application.camera.CalculateViewMatrix();
  const spitfire::math::cVec3 camx = matView.GetColumn(0).GetVec3();
  const spitfire::math::cVec3 camz = matView.GetColumn(2).GetVec3();
  float camrot = 4.0f * (spitfire::math::v3Back.DotProduct(camx) + spitfire::math::v3Up.DotProduct(camz));

  const spitfire::math::cMat3 starRotationMatOriginal(
    cosf(camrot) * 0.5f, -sinf(camrot), 0.0f,
    sinf(camrot), cosf(camrot) * 0.5f, 0.0f,
    0.0f, 0.0f, 1.0f
  );
  const spitfire::math::cMat3 sb1(
    2.0f, 0.0f, -1.0f,
    0.0f, 2.0f, -1.0f,
    0.0f, 0.0f, 1.0f
    );
  const spitfire::math::cMat3 sb2(
    0.5f, 0.0f, 0.5f,
    0.0f, 0.5f, 0.5f,
    0.0f, 0.0f, 1.0f
    );
  const spitfire::math::cMat3 starRotationMat = sb2 * starRotationMatOriginal * sb1;

  context.BeginRenderToTexture(fboOut);

  context.BindShader(*shaderPostProcess_);

  context.SetShaderConstant("uLensStarMatrix", starRotationMat);

  context.BindTexture(0, fboIn);
  context.BindTexture(1, *fboTempC_);
  context.BindTexture(2, *texLensDirt_);
  context.BindTexture(3, *texLensStar_);

  application.RenderScreenRectangleShaderAndTextureAlreadySet();

  context.UnBindTexture(3, *texLensStar_);
  context.UnBindTexture(2, *texLensDirt_);
  context.UnBindTexture(1, *fboTempC_);
  context.UnBindTexture(0, fboIn);

  context.UnBindShader(*shaderPostProcess_);

  context.EndRenderToTexture(fboOut);
}

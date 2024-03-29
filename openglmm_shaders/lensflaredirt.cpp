#include <cassert>
#include <cmath>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>
#include <list>

#ifdef __WIN__
// OpenGL headers
#include <GL/GLee.h>
#include <GL/glu.h>
#endif

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
  flareSamples_(14),
  flareDispersal_(0.3f),
  flareHaloWidth_(0.6f),
  flareDistortion_(10.5f),
  uiFlareBlurRadius(24),
  tempBufferSize_(12),
  setTempBufferSize_(12.0f)
{
}

void cLensFlareDirt::Init(cApplication& application, opengl::cContext& context)
{
  //------------------------------------------------------------------------------
  // TEXTURES
  //------------------------------------------------------------------------------

  context.CreateTexture(texLensDirt_, TEXT("textures/lensdirt.png"));
  ASSERT(texLensDirt_.IsValid());
  texLensDirt_.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  texLensDirt_.SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  context.CreateTexture(texLensStar_, TEXT("textures/lensstar.png"));
  ASSERT(texLensStar_.IsValid());
  texLensStar_.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  texLensStar_.SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  context.CreateTexture(texLensColor_, TEXT("textures/lenscolor.png"));
  ASSERT(texLensColor_.IsValid());
  texLensColor_.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  texLensColor_.SetWrap(opengl::TEXTURE_WRAP::REPEAT);



  //------------------------------------------------------------------------------
  // SHADERS
  //------------------------------------------------------------------------------
  context.CreateShader(shaderScaleBias_, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/lensflaredirt/scalebias.frag"));
  ASSERT(shaderScaleBias_.IsCompiledProgram());
  context.CreateShader(shaderLensflare_, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/lensflaredirt/lensflare.frag"));
  ASSERT(shaderLensflare_.IsCompiledProgram());
  context.CreateShader(shaderPostProcess_, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/lensflaredirt/postprocess.frag"));
  ASSERT(shaderPostProcess_.IsCompiledProgram());


  CreateTempBuffers(application, context);
}

void cLensFlareDirt::Destroy(opengl::cContext& context)
{
  DestroyTempBuffers(context);


  if (shaderPostProcess_.IsCompiledProgram()) context.DestroyShader(shaderPostProcess_);
  if (shaderScaleBias_.IsCompiledProgram()) context.DestroyShader(shaderScaleBias_);
  if (shaderLensflare_.IsCompiledProgram()) context.DestroyShader(shaderLensflare_);


  if (texLensDirt_.IsValid()) context.DestroyTexture(texLensDirt_);
  if (texLensStar_.IsValid()) context.DestroyTexture(texLensStar_);
  if (texLensColor_.IsValid()) context.DestroyTexture(texLensColor_);
}

void cLensFlareDirt::CreateTempBuffers(cApplication& application, opengl::cContext& context)
{
  const size_t width = context.GetWidth();
  const size_t height = context.GetHeight();

  const size_t fboWidth = width / tempBufferSize_;
  const size_t fboHeight = height / tempBufferSize_;

  // Frame buffer objects
  context.CreateTextureFrameBufferObjectWithDepth(fboTempA_, fboWidth, fboHeight);
  ASSERT(fboTempA_.IsValid());
  fboTempA_.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempA_.SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  context.CreateTextureFrameBufferObjectWithDepth(fboTempB_, fboWidth, fboHeight);
  ASSERT(fboTempB_.IsValid());
  fboTempB_.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempB_.SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  context.CreateTextureFrameBufferObjectWithDepth(fboTempC_, fboWidth, fboHeight);
  ASSERT(fboTempC_.IsValid());
  fboTempC_.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempC_.SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  // VBOs
  application.CreateScreenRectVBO(vboScaleBias, 1.0f, 1.0f, width, height);
  application.CreateScreenRectVBO(vboLensFlare, 1.0f, 1.0f, fboWidth, fboHeight);


  blur.Init(application, context, fboWidth, fboHeight);
}

void cLensFlareDirt::DestroyTempBuffers(opengl::cContext& context)
{
  // VBOs
  context.DestroyStaticVertexBufferObject(vboLensFlare);
  context.DestroyStaticVertexBufferObject(vboScaleBias);

  // Frame buffer objects
  if (fboTempA_.IsValid()) context.DestroyTextureFrameBufferObject(fboTempA_);
  if (fboTempB_.IsValid()) context.DestroyTextureFrameBufferObject(fboTempB_);
  if (fboTempC_.IsValid()) context.DestroyTextureFrameBufferObject(fboTempC_);


  blur.Destroy(context);
}

void cLensFlareDirt::Resize(opengl::cContext& context)
{
  blur.Resize(context);
}

void cLensFlareDirt::Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& fboIn, opengl::cTextureFrameBufferObject& fboOut, float fExposure, bool bDebugShowFlareOnly)
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
  context.BeginRenderToTexture(fboTempA_);
  context.BindShader(shaderScaleBias_);
  context.SetShaderConstant("uScale", spitfire::math::cVec3(flareScale_));
  context.SetShaderConstant("uBias", spitfire::math::cVec3(flareBias_));
  context.BindTexture(0, fboIn);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboScaleBias);
  context.UnBindTexture(0, fboIn);
  context.UnBindShader(shaderScaleBias_);
  context.EndRenderToTexture(fboTempA_);

  // feature generation:
  context.BeginRenderToTexture(fboTempB_);
  context.BindShader(shaderLensflare_);
  context.SetShaderConstant("uSamples", flareSamples_);
  context.SetShaderConstant("uDispersal", flareDispersal_);
  context.SetShaderConstant("uHaloWidth", flareHaloWidth_);
  context.SetShaderConstant("uDistortion", flareDistortion_);

  // Adjust the exposure
  const float fLensFlareExposure = fExposure + 2.0f;
  context.SetShaderConstant("fExposure", 1.0f / fLensFlareExposure);

  context.BindTexture(0, fboTempA_);
  context.BindTexture(1, texLensColor_);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboLensFlare);
  context.UnBindTexture(1, texLensColor_);
  context.UnBindTexture(0, fboTempA_);
  context.UnBindShader(shaderLensflare_);
  context.EndRenderToTexture(fboTempB_);

  //	Apply gaussian blur
  blur.Render(application, context, fboTempB_, fboTempA_, fboTempC_, uiFlareBlurRadius);


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

  context.BindShader(shaderPostProcess_);

  context.SetShaderConstant("matCameraLensStarBurst", matStar);
  context.SetShaderConstant("fSceneMix0To1", bDebugShowFlareOnly ? 0.0f : 1.0f);
  context.SetShaderConstant("fFlareMix0To1", 2.0f);

  context.BindTexture(0, fboIn);
  context.BindTexture(1, fboTempC_);
  context.BindTexture(2, texLensDirt_);
  context.BindTexture(3, texLensStar_);

  application.RenderScreenRectangleShaderAndTextureAlreadySet(vboScaleBias);

  context.UnBindTexture(3, texLensStar_);
  context.UnBindTexture(2, texLensDirt_);
  context.UnBindTexture(1, fboTempC_);
  context.UnBindTexture(0, fboIn);

  context.UnBindShader(shaderPostProcess_);

  context.EndRenderToTexture(fboOut);
}

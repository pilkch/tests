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
  exposure_(1.0f),
  bloomScale_(0.05f), bloomBias_(-1.0f),
  bloomBlurRadius_(64.0f),
  flareSamples_(8.0f), flareDispersal_(0.3f), flareHaloWidth_(0.6f), flareDistortion_(1.5f),
  flareScale_(0.05f), flareBias_(-3.5f),
  flareBlurRadius_(24.0f),
  flareOnly_(false),
  tempBufferSize_(4), setTempBufferSize_(4.0f),
  currentAdaptLuma_(0),
  adaptionRate_(2.5f)
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
  shaderPostProcess_ = context.CreateShader(TEXT("shaders/basic.vs.glsl"), TEXT("shaders/postprocess.fs.glsl"));
  ASSERT(shaderPostProcess_ != nullptr);
  shaderLuminance_ = context.CreateShader(TEXT("shaders/basic.vs.glsl"), TEXT("shaders/lum.fs.glsl"));
  ASSERT(shaderLuminance_ != nullptr);
  shaderAvgMinMax_ = context.CreateShader(TEXT("shaders/basic.vs.glsl"), TEXT("shaders/avgminmax.fs.glsl"));
  ASSERT(shaderAvgMinMax_ != nullptr);
  shaderLuminanceAdapt_ = context.CreateShader(TEXT("shaders/basic.vs.glsl"), TEXT("shaders/adaptlum.fs.glsl"));
  ASSERT(shaderLuminanceAdapt_ != nullptr);
  shaderGaussBlur_ = context.CreateShader(TEXT("shaders/basic.vs.glsl"), TEXT("shaders/gauss1d.fs.glsl"));
  ASSERT(shaderGaussBlur_ != nullptr);
  shaderScaleBias_ = context.CreateShader(TEXT("shaders/basic.vs.glsl"), TEXT("shaders/scalebias.fs.glsl"));
  ASSERT(shaderScaleBias_ != nullptr);
  shaderLensflare_ = context.CreateShader(TEXT("shaders/basic.vs.glsl"), TEXT("shaders/lensflare.fs.glsl"));
  ASSERT(shaderLensflare_ != nullptr);

  //------------------------------------------------------------------------------
  //	FRAMEBUFFERS
  //------------------------------------------------------------------------------
  //	HDR RENDER BUFFER
  const size_t width = context.GetWidth();
  const size_t height = context.GetHeight();
  fboHdr_ = context.CreateTextureFrameBufferObjectWithDepth(width, height);
  ASSERT(fboHdr_ != nullptr);
  fboHdr_->SetMagFilter(opengl::TEXTURE_FILTER::NEAREST);
  fboHdr_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  //	AUX RENDER BUFFERS
  fboTempA_ = context.CreateTextureFrameBufferObject(width / tempBufferSize_, height / tempBufferSize_);
  ASSERT(fboTempA_ != nullptr);
  fboTempA_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempA_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  fboTempB_ = context.CreateTextureFrameBufferObject(width / tempBufferSize_, height / tempBufferSize_);
  ASSERT(fboTempB_ != nullptr);
  fboTempB_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempB_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  fboTempC_ = context.CreateTextureFrameBufferObject(width / tempBufferSize_, height / tempBufferSize_);
  ASSERT(fboTempC_ != nullptr);
  fboTempC_->SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboTempC_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  // AUTO EXPOSURE BUFFERS
  fboLuma_ = CreateFBO(GL_RGB16F, 256, 256);
  ASSERT(fboLuma_ != nullptr);
  fboLuma_->SetMinFilter(opengl::TEXTURE_FILTER::LINEAR_MIPMAP_NEAREST); // no filtering between mip levels
  fboLuma_->SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);
  fboLuma_->generateMipmap(); // allocate mipmap

  for (size_t i = 0; i < 2; ++i) {
    fboAdaptLuma_[i] = CreateFBO(GL_R16F, 1, 1);
    ASSERT(fboAdaptLuma_[i] != nullptr);
    fboAdaptLuma_[i]->SetMinFilter(opengl::TEXTURE_FILTER::NEAREST); // don't want filtering between mip levels!
    fboAdaptLuma_[i]->SetMagFilter(opengl::TEXTURE_FILTER::NEAREST); // don't want filtering between mip levels!
    fboAdaptLuma_[i]->SetWrap(opengl::TEXTURE_WRAP::REPEAT);
  }
}

void cLensFlareDirt::Destroy(opengl::cContext& context)
{
  if (pShaderRenderToDepthTexture != nullptr) {
    context.DestroyShader(pShaderRenderToDepthTexture);
    pShaderRenderToDepthTexture = nullptr;
  }
  if (pShaderShadowMap != nullptr) {
    context.DestroyShader(pShaderShadowMap);
    pShaderShadowMap = nullptr;
  }
  if (pTextureDepthTexture != nullptr) {
    context.DestroyTextureFrameBufferObject(pTextureDepthTexture);
    pTextureDepthTexture = nullptr;
  }
}

void cLensFlareDirt::gaussBlur(
  opengl::cTextureFrameBufferObject* in,
  opengl::cTextureFrameBufferObject* temp,
  opengl::cTextureFrameBufferObject* out,
  int radius
  )
{
  assert(in);
  const oogl::Texture *texIn = in->getAttachment(GL_COLOR_ATTACHMENT0);
  assert(texIn);

  assert(temp);
  const oogl::Texture *texTemp = temp->getAttachment(GL_COLOR_ATTACHMENT0);
  assert(texTemp);

  shaderGaussBlur_->use();
  OOGL_CALL(glUniform1i(shaderGaussBlur_->getUniformLocation("uBlurRadius"), radius));
  OOGL_CALL(glActiveTexture(GL_TEXTURE0));

  // horizontal pass:
  temp->bindDraw();
  oogl::viewport(temp);
  OOGL_CALL(glUniform2f(shaderGaussBlur_->getUniformLocation("uBlurDirection"), 1.0f, 0.0f));
  texIn->bind();
  frm::modelSAQuad->enable(shaderGaussBlur_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);

  // vertical pass:
  assert(out);
  out->bindDraw();
  oogl::viewport(out);
  OOGL_CALL(glUniform2f(shaderGaussBlur_->getUniformLocation("uBlurDirection"), 0.0f, 1.0f));
  texTemp->bind();
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);

  //	restore state:
  frm::modelSAQuad->disable();
  frm::Shader::useNone();
  frm::Framebuffer::bindNone();
}

void cLensFlareDirt::renderToMipmap(
  opengl::cTextureFrameBufferObject* fbo,
  opengl::cShader *shader,
  int bindLocation
  )
{
  assert(fbo);

  oogl::Texture2D *tex = (oogl::Texture2D*)fbo->getAttachment(GL_COLOR_ATTACHMENT0);
  assert(tex);

  sml::vec2i texSize(tex->getWidth(), tex->getHeight());
  int nMipLevels = (int)sml::log((float)texSize.y(), 2.0f);

  assert(shader);
  frm::modelSAQuad->enable(shader, this);
  fbo->bindDraw();
  glActiveTexture(GL_TEXTURE0 + bindLocation);
  tex->bind();

  for (int i = 1; i <= nMipLevels; ++i) {
    fbo->attach(tex, GL_COLOR_ATTACHMENT0, i);
    tex->setBaseLevel(i - 1);
    tex->setMaxLevel(i - 1);
    texSize /= 2;
    OOGL_CALL(glViewport(0, 0, texSize.x(), texSize.y()));
    frm::modelSAQuad->render();
  }

  //	restore mip min/max level:
  fbo->attach(tex, GL_COLOR_ATTACHMENT0, 0); // attach the top layer
  tex->setBaseLevel(0);
  tex->setMaxLevel(nMipLevels);

  frm::modelSAQuad->disable();
  frm::Framebuffer::bindNone();
}

bool cLensFlareDirt::resizeTempBuffers()
{
  const size_t width = context.GetWidth();
  const size_t height = context.GetHeight();

  // Resize textures
  fboTempA_->bind();
  fboTempA_->image(0, fboTempA_->getFormat(), width / tempBufferSize_, height / tempBufferSize_);
  fboTempB_->bind();
  fboTempB_->image(0, fboTempB_->getFormat(), width / tempBufferSize_, height / tempBufferSize_);
  fboTempC_->bind();
  fboTempC_->image(0, fboTempC_->getFormat(), width / tempBufferSize_, height / tempBufferSize_);
  frm::Texture2d::bindNone();

  //	rebind to fbos - this is only necessary so that OOGL can recompute the
  //	attachment properties:
  fboTempA_->bind();
  fboTempA_->attach(fboTempA_, GL_COLOR_ATTACHMENT0);
  fboTempB_->bind();
  fboTempB_->attach(fboTempB_, GL_COLOR_ATTACHMENT0);
  fboTempC_->bind();
  fboTempC_->attach(fboTempC_, GL_COLOR_ATTACHMENT0);
  frm::Framebuffer::bindNone();

  return true;
}

void cLensFlareDirt::Resize(cApplication& application, opengl::cContext& context)
{
}

void cLensFlareDirt::Render(cApplication& application, float dt, opengl::cContext& context, opengl::cTextureFrameBufferObject& fboIn, opengl::cTextureFrameBufferObject& fboOut)
{
  if ((int)setTempBufferSize_ != tempBufferSize_) {
    tempBufferSize_ = (int)setTempBufferSize_;
    if (!resizeTempBuffers())
      return;
  }

  frm::Camera &camera = *getCurrentCamera();
  frm::FirstPersonCtrl &fpCtrl = *fpCtrls_[currentFpCtrl_];
  int previousAdaptLuma = currentAdaptLuma_;
  currentAdaptLuma_ = (currentAdaptLuma_ + 1) % 2;

  //---------------------------------------------------------------------------
  //	GENERATE LENS FLARE
  //---------------------------------------------------------------------------
    
  //	downsample/threshold:
  fboTempA_->bindDraw();
  oogl::viewport(fboTempA_);
  shaderScaleBias_->use();
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uScale"), 1, sml::vec4f(flareScale_)));
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uBias"), 1, sml::vec4f(flareBias_)));
  OOGL_CALL(glActiveTexture(GL_TEXTURE0));
  fboHdr_->bind();
  frm::modelSAQuad->enable(shaderScaleBias_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();

  // feature generation:
  fboTempB_->bindDraw();
  shaderLensflare_->use();
  OOGL_CALL(glUniform1i(shaderLensflare_->getUniformLocation("uSamples"), (int)flareSamples_));
  OOGL_CALL(glUniform1f(shaderLensflare_->getUniformLocation("uDispersal"), flareDispersal_));
  OOGL_CALL(glUniform1f(shaderLensflare_->getUniformLocation("uHaloWidth"), flareHaloWidth_));
  OOGL_CALL(glUniform1f(shaderLensflare_->getUniformLocation("uDistortion"), flareDistortion_));
  OOGL_CALL(glActiveTexture(GL_TEXTURE0));
  fboTempA_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE1));
  texLensColor_->bind(); bind 1D?
  frm::modelSAQuad->enable(shaderLensflare_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();

  //	gaussian blur:
  gaussBlur(fboTempB_, fboTempA_, fboTempC_, (int)flareBlurRadius_);

  //	can't upscale/blend as per bloom; we need to apply the lens flare after
  //	motion blur, etc. in the final post process


  //---------------------------------------------------------------------------
  //	FINAL POST PROCESS (TONEMAP, MBLUR, APPLY LENSFLARE, ETC.):
  //---------------------------------------------------------------------------
  //	matrix inputs for camera mblur:
  static sml::mat4f prevViewProjMat = camera.getViewProjectionMatrix();
  sml::mat4f inverseViewProjMat = sml::inverse(camera.getViewProjectionMatrix());

  //	transformation matrix for lens flare starburst:
  sml::vec3f camx = sml::vec3f(camera.getViewMatrix().col(0));
  sml::vec3f camz = sml::vec3f(camera.getViewMatrix().col(2));
  float camrot = sml::dot(camx, sml::vec3f::zAxis()) + sml::dot(camz, sml::vec3f::yAxis());
  camrot *= 4.0f;
  sml::mat3f starRotationMat(
    sml::cos(camrot) * 0.5f, -sml::sin(camrot), 0.0f,
    sml::sin(camrot), sml::cos(camrot) * 0.5f, 0.0f,
    0.0f, 0.0f, 1.0f
    );
  sml::mat3f sb1(
    2.0f, 0.0f, -1.0f,
    0.0f, 2.0f, -1.0f,
    0.0f, 0.0f, 1.0f
    );
  sml::mat3f sb2(
    0.5f, 0.0f, 0.5f,
    0.0f, 0.5f, 0.5f,
    0.0f, 0.0f, 1.0f
    );
  starRotationMat = sb2 * starRotationMat * sb1;

  fboDefault_->bindDraw();
  oogl::viewport(fboDefault_);
  shaderPostProcess_->use();
  OOGL_CALL(glUniform1f(shaderPostProcess_->getUniformLocation("uExposure"), exposure_));
  OOGL_CALL(glUniform1f(shaderPostProcess_->getUniformLocation("uFrameTime"), dt));
  OOGL_CALL(glUniformMatrix4fv(shaderPostProcess_->getUniformLocation("uInverseViewProjectionMatrix"), 1, GL_FALSE, inverseViewProjMat));
  OOGL_CALL(glUniformMatrix4fv(shaderPostProcess_->getUniformLocation("uPrevViewProjectionMatrix"), 1, GL_FALSE, prevViewProjMat));
  OOGL_CALL(glUniform1f(shaderPostProcess_->getUniformLocation("uMotionScale"), (1.0f / 60.0f) / dt));

  OOGL_CALL(glUniformMatrix3fv(shaderPostProcess_->getUniformLocation("uLensStarMatrix"), 1, GL_FALSE, starRotationMat));
  OOGL_CALL(glUniform1i(shaderPostProcess_->getUniformLocation("uFlareOnly"), (int)flareOnly_));

  OOGL_CALL(glActiveTexture(GL_TEXTURE0));
  fboHdr_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE1));
  fboHdr_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE2));
  texAdaptLuma_[currentAdaptLuma_]->bind();

  OOGL_CALL(glActiveTexture(GL_TEXTURE3));
  fboTempC_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE4));
  texLensDirt_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE5));
  texLensStar_->bind();


  frm::modelSAQuad->enable(shaderPostProcess_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();

  prevViewProjMat = camera.getViewProjectionMatrix();
}

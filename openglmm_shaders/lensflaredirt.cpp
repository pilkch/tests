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

// Breathe headers
#include <breathe/render/model/cFileFormatOBJ.h>
#include <breathe/render/model/cStatic.h>

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
  uShadowMapTextureSize(2048),
  pTextureDepthTexture(nullptr),
  pShaderShadowMap(nullptr),
  pShaderRenderToDepthTexture(nullptr)
{
}

void cLensFlareDirt::Init(opengl::cContext& context)
{
  pShaderRenderToDepthTexture = context.CreateShader(TEXT("shaders/depthrendertotexture.vert"), TEXT("shaders/depthrendertotexture.frag"));
  assert(pShaderRenderToDepthTexture != nullptr);
  pShaderShadowMap = context.CreateShader(TEXT("shaders/shadowmapping.vert"), TEXT("shaders/shadowmapping.frag"));
  assert(pShaderShadowMap != nullptr);
  pTextureDepthTexture = context.CreateTextureFrameBufferObjectDepthShadowOnly(uShadowMapTextureSize, uShadowMapTextureSize);
  assert(pTextureDepthTexture != nullptr);
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


cLensFlareDirt::cLensFlareDirt(
  const frm::CStringT &appName,
  const frm::ivec2 &windowSize,
  const frm::ivec2 &renderSize,
  bool fullscreen
  ) : App3d(appName, windowSize, renderSize, fullscreen),
  exposure_(1.0f),
  bloomScale_(0.05f), bloomBias_(-1.0f),
  bloomBlurRadius_(64.0f),
  flareSamples_(8.0f), flareDispersal_(0.3f), flareHaloWidth_(0.6f), flareDistortion_(1.5f),
  flareScale_(0.05f), flareBias_(-3.5f),
  flareBlurRadius_(24.0f),
  flareOnly_(false),
  auxBufferSize_(4), setAuxBufferSize_(4.0f),
  currentAdaptLuma_(0),
  adaptionRate_(2.5f)
{
}

/*----------------------------------------------------------------------------*/
bool cLensFlareDirt::initScene() {
  //------------------------------------------------------------------------------
  //	TEXTURES
  //------------------------------------------------------------------------------

  texLensDirt_ = frm::Texture2d::loadFromFile("media/lensdirt.png");
  if (!texLensDirt_)
    return false;
  texLensDirt_->setMagFilter(GL_LINEAR);
  texLensDirt_->setWrap(GL_CLAMP_TO_EDGE);

  texLensStar_ = frm::Texture2d::loadFromFile("media/lensstar.png");
  if (!texLensStar_)
    return false;
  texLensStar_->setMagFilter(GL_LINEAR);
  texLensStar_->setWrap(GL_CLAMP_TO_EDGE);

  texLensColor_ = frm::Texture1d::loadFromFile("media/lenscolor.png");
  if (!texLensColor_)
    return false;
  texLensColor_->setMagFilter(GL_LINEAR);
  texLensColor_->setWrap(GL_REPEAT);

  return true;
}

/*----------------------------------------------------------------------------*/
bool cLensFlareDirt::initGui() {
  if (!App3d::initGui())
    return false;

  frm::GuiListContainer *sliderList = frm::GuiListContainer::create();
  sliderList->setDirection(frm::IGuiComponent::VERTICAL);
  sliderList->setAlignment(frm::IGuiComponent::RIGHT, frm::IGuiComponent::TOP);
  sliderList->setSizeRef(frm::IGuiComponent::ABSOLUTE, frm::IGuiComponent::RELATIVE);
  sliderList->setPosition(sml::vec2i(20, 0));
  sliderList->setSize(sml::vec2i(260, 100));
  addGuiComponent(sliderList);

  /*addGuiComponent(
  frm::GuiSlider::create(
  &exposure_,
  frm::ivec2(0.0f, 2.0f),
  frm::Texture2d::loadFromFile("media/gui/slider16.png"),
  frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
  frm::fontDefault,
  "Exposure %2.2f"
  ),
  sliderList
  );

  addGuiComponent(
  frm::GuiSlider::create(
  &bloomScale_,
  frm::ivec2(0.0f, 1.0f),
  frm::Texture2d::loadFromFile("media/gui/slider16.png"),
  frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
  frm::fontDefault,
  "Bloom scale %1.2f"
  ),
  sliderList
  );
  addGuiComponent(
  frm::GuiSlider::create(
  &bloomBias_,
  frm::ivec2(-50.0f, 0.0f),
  frm::Texture2d::loadFromFile("media/gui/slider16.png"),
  frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
  frm::fontDefault,
  "Bloom bias %1.2f"
  ),
  sliderList
  );
  addGuiComponent(
  frm::GuiSlider::create(
  &bloomBlurRadius_,
  frm::ivec2(0.0f, 128.0f),
  frm::Texture2d::loadFromFile("media/gui/slider16.png"),
  frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
  frm::fontDefault,
  "Bloom blur size %3.0f"
  ),
  sliderList
  );
  addGuiComponent(
  frm::GuiSlider::create(
  &adaptionRate_,
  frm::ivec2(0.0f, 10.0f),
  frm::Texture2d::loadFromFile("media/gui/slider16.png"),
  frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
  frm::fontDefault,
  "Adaption rate %2.2f"
  ),
  sliderList
  );*/

  addGuiComponent(
    frm::GuiSlider::create(
    &setAuxBufferSize_,
    frm::ivec2(1.0f, 16.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Downsample 1/%1.0f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiSlider::create(
    &flareScale_,
    frm::ivec2(0.0f, 1.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Flare scale %1.2f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiSlider::create(
    &flareBias_,
    frm::ivec2(-50.0f, 0.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Flare bias %1.2f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiSlider::create(
    &flareSamples_,
    frm::ivec2(1.0f, 24.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Ghost samples %1.0f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiSlider::create(
    &flareDispersal_,
    frm::ivec2(0.0f, 2.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Ghost dispersal %1.2f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiSlider::create(
    &flareHaloWidth_,
    frm::ivec2(0.0f, 10.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Halo size %1.2f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiSlider::create(
    &flareDistortion_,
    frm::ivec2(0.0f, 16.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Chroma distortion %1.2f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiSlider::create(
    &flareBlurRadius_,
    frm::ivec2(0.0f, 128.0f),
    frm::Texture2d::loadFromFile("media/gui/slider16.png"),
    frm::Texture2d::loadFromFile("media/gui/sliderKnob16.png"),
    frm::fontDefault,
    "Blur size %3.0f"
    ),
    sliderList
    );
  addGuiComponent(
    frm::GuiCheckbox::create(
    &flareOnly_,
    frm::Texture2d::loadFromFile("media/gui/checkbox16off.png"),
    frm::Texture2d::loadFromFile("media/gui/checkbox16on.png"),
    frm::fontDefault,
    "Show flare only"
    ),
    sliderList
    );

  sliderList->setDirection(frm::IGuiComponent::VERTICAL); // KLUDGE

  return true;
}

/*----------------------------------------------------------------------------*/
bool cLensFlareDirt::initRender() {
  if (!App3d::initRender())
    return false;

  //------------------------------------------------------------------------------
  //	SHADERS
  //------------------------------------------------------------------------------
  shaderPostProcess_ = frm::Shader::loadFromFiles("shaders/basic.vs.glsl", "shaders/postprocess.fs.glsl");
  if (!shaderPostProcess_)
    return false;
  shaderLuminance_ = frm::Shader::loadFromFiles("shaders/basic.vs.glsl", "shaders/lum.fs.glsl");
  if (!shaderLuminance_)
    return false;
  shaderAvgMinMax_ = frm::Shader::loadFromFiles("shaders/basic.vs.glsl", "shaders/avgminmax.fs.glsl");
  if (!shaderAvgMinMax_)
    return false;
  shaderLuminanceAdapt_ = frm::Shader::loadFromFiles("shaders/basic.vs.glsl", "shaders/adaptlum.fs.glsl");
  if (!shaderLuminanceAdapt_)
    return false;
  shaderGaussBlur_ = frm::Shader::loadFromFiles("shaders/basic.vs.glsl", "shaders/gauss1d.fs.glsl");
  if (!shaderGaussBlur_)
    return false;
  shaderScaleBias_ = frm::Shader::loadFromFiles("shaders/basic.vs.glsl", "shaders/scalebias.fs.glsl");
  if (!shaderScaleBias_)
    return false;
  shaderLensflare_ = frm::Shader::loadFromFiles("shaders/basic.vs.glsl", "shaders/lensflare.fs.glsl");
  if (!shaderLensflare_)
    return false;

  //------------------------------------------------------------------------------
  //	FRAMEBUFFERS
  //------------------------------------------------------------------------------
  /*	HDR RENDER BUFFER */
  texHdr_ = frm::Texture2d::create(GL_RGBA16F, renderSize_.x(), renderSize_.y());
  if (!texHdr_)
    return false;
  texHdr_->setMagFilter(GL_NEAREST);
  texHdr_->setWrap(GL_CLAMP_TO_EDGE);
  texHdr_->setName("HDR");

  texDepth_ = frm::Texture2d::create(GL_DEPTH_COMPONENT, renderSize_.x(), renderSize_.y());
  if (!texDepth_)
    return false;
  texDepth_->setWrap(GL_CLAMP_TO_EDGE);
  texDepth_->setName("DEPTH");

  fboHdr_ = frm::Framebuffer::create();
  if (!fboHdr_)
    return false;
  fboHdr_->attach(texHdr_, GL_COLOR_ATTACHMENT0);
  fboHdr_->attach(texDepth_, GL_DEPTH_ATTACHMENT);

  /*	AUX RENDER BUFFERS */
  texAuxA_ = frm::Texture2d::create(GL_RGBA16F, renderSize_.x() / auxBufferSize_, renderSize_.y() / auxBufferSize_);
  if (!texAuxA_)
    return false;
  texAuxA_->setMagFilter(GL_LINEAR);
  texAuxA_->setWrap(GL_CLAMP_TO_EDGE);
  texAuxA_->setName("AUX A");
  fboAuxA_ = frm::Framebuffer::create();
  if (!fboAuxA_)
    return false;
  fboAuxA_->attach(texAuxA_, GL_COLOR_ATTACHMENT0);

  texAuxB_ = frm::Texture2d::create(GL_RGBA16F, renderSize_.x() / auxBufferSize_, renderSize_.y() / auxBufferSize_);
  texAuxB_->setMagFilter(GL_LINEAR);
  texAuxB_->setWrap(GL_CLAMP_TO_EDGE);
  texAuxB_->setName("AUX B");
  if (!texAuxB_)
    return false;
  fboAuxB_ = frm::Framebuffer::create();
  if (!fboAuxB_)
    return false;
  fboAuxB_->attach(texAuxB_, GL_COLOR_ATTACHMENT0);

  texAuxC_ = frm::Texture2d::create(GL_RGBA16F, renderSize_.x() / auxBufferSize_, renderSize_.y() / auxBufferSize_);
  if (!texAuxC_)
    return false;
  texAuxC_->setMagFilter(GL_LINEAR);
  texAuxC_->setWrap(GL_CLAMP_TO_EDGE);
  texAuxC_->setName("AUX C");

  fboAuxC_ = frm::Framebuffer::create();
  if (!fboAuxC_)
    return false;
  fboAuxC_->attach(texAuxC_, GL_COLOR_ATTACHMENT0);

  /*	AUTO EXPOSURE BUFFERS */
  texLuma_ = frm::Texture2d::create(GL_RGB16F, 256, 256);
  if (!texLuma_)
    return false;
  texLuma_->setMinFilter(GL_LINEAR_MIPMAP_NEAREST); // no filtering between mip levels
  texLuma_->setWrap(GL_CLAMP_TO_EDGE);
  texLuma_->generateMipmap(); // allocate mipmap
  fboLuma_ = frm::Framebuffer::create();
  if (!fboLuma_)
    return false;
  fboLuma_->attach(texLuma_, GL_COLOR_ATTACHMENT0);

  for (int i = 0; i < 2; ++i) {
    texAdaptLuma_[i] = frm::Texture2d::create(GL_R16F, 1, 1);
    if (!texAdaptLuma_[i])
      return false;
    texAdaptLuma_[i]->setMinFilter(GL_NEAREST); // don't want filtering between mip levels!
    texAdaptLuma_[i]->setMagFilter(GL_NEAREST); // don't want filtering between mip levels!
    texAdaptLuma_[i]->setWrap(GL_REPEAT);
    fboAdaptLuma_[i] = frm::Framebuffer::create();
    if (!fboAdaptLuma_[i])
      return false;
    fboAdaptLuma_[i]->attach(texAdaptLuma_[i], GL_COLOR_ATTACHMENT0);
  }

  //	add render textures to visualize for overlay:
  addRenderTexture(texHdr_);
  addRenderTexture(texAuxA_);
  addRenderTexture(texAuxB_);
  addRenderTexture(texAuxC_);

  //------------------------------------------------------------------------------
  //	OPENGL STATE
  //------------------------------------------------------------------------------
  OOGL_CALL(glDisable(GL_DEPTH_TEST));
  OOGL_CALL(glFrontFace(GL_CCW)); // default is GL_CCW
  OOGL_CALL(glCullFace(GL_BACK));
  OOGL_CALL(glEnable(GL_CULL_FACE));
  OOGL_CALL(glClearDepth(1.0f));
  OOGL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    
  return true;
}

/*----------------------------------------------------------------------------*/
void cLensFlareDirt::gaussBlur(
  frm::Framebuffer *in,
  frm::Framebuffer *aux,
  frm::Framebuffer *out,
  int radius
  ) {
  assert(in);
  const oogl::Texture *texIn = in->getAttachment(GL_COLOR_ATTACHMENT0);
  assert(texIn);

  assert(aux);
  const oogl::Texture *texAux = aux->getAttachment(GL_COLOR_ATTACHMENT0);
  assert(texAux);

  shaderGaussBlur_->use();
  OOGL_CALL(glUniform1i(shaderGaussBlur_->getUniformLocation("uBlurRadius"), radius));
  OOGL_CALL(glActiveTexture(GL_TEXTURE0));

  // horizontal pass:
  aux->bindDraw();
  oogl::viewport(aux);
  OOGL_CALL(glUniform2f(shaderGaussBlur_->getUniformLocation("uBlurDirection"), 1.0f, 0.0f));
  texIn->bind();
  frm::modelSAQuad->enable(shaderGaussBlur_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);

  // vertical pass:
  assert(out);
  out->bindDraw();
  oogl::viewport(out);
  OOGL_CALL(glUniform2f(shaderGaussBlur_->getUniformLocation("uBlurDirection"), 0.0f, 1.0f));
  texAux->bind();
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);

  //	restore state:
  frm::modelSAQuad->disable();
  frm::Shader::useNone();
  frm::Framebuffer::bindNone();
}

/*----------------------------------------------------------------------------*/
void cLensFlareDirt::renderToMipmap(
  frm::Framebuffer *fbo,
  frm::Shader *shader,
  int bindLocation
  ) {
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

/*----------------------------------------------------------------------------*/
bool cLensFlareDirt::resizeAuxBuffers() {
  //	resize textures:
  texAuxA_->bind();
  texAuxA_->image(0, texAuxA_->getFormat(), renderSize_.x() / auxBufferSize_, renderSize_.y() / auxBufferSize_);
  texAuxB_->bind();
  texAuxB_->image(0, texAuxB_->getFormat(), renderSize_.x() / auxBufferSize_, renderSize_.y() / auxBufferSize_);
  texAuxC_->bind();
  texAuxC_->image(0, texAuxC_->getFormat(), renderSize_.x() / auxBufferSize_, renderSize_.y() / auxBufferSize_);
  frm::Texture2d::bindNone();

  //	rebind to fbos - this is only necessary so that OOGL can recompute the
  //	attachment properties:
  fboAuxA_->bind();
  fboAuxA_->attach(texAuxA_, GL_COLOR_ATTACHMENT0);
  fboAuxB_->bind();
  fboAuxB_->attach(texAuxB_, GL_COLOR_ATTACHMENT0);
  fboAuxC_->bind();
  fboAuxC_->attach(texAuxC_, GL_COLOR_ATTACHMENT0);
  frm::Framebuffer::bindNone();

  return true;
}

/*----------------------------------------------------------------------------*/
bool cLensFlareDirt::render(float dt)
{
  if ((int)setAuxBufferSize_ != auxBufferSize_) {
    auxBufferSize_ = (int)setAuxBufferSize_;
    if (!resizeAuxBuffers())
      return false;
  }

  frm::Camera &camera = *getCurrentCamera();;
  frm::FirstPersonCtrl &fpCtrl = *fpCtrls_[currentFpCtrl_];
  int previousAdaptLuma = currentAdaptLuma_;
  currentAdaptLuma_ = (currentAdaptLuma_ + 1) % 2;

  //---------------------------------------------------------------------------
  //	RENDER SCENE TO HDR BUFFER
  //---------------------------------------------------------------------------
  fboHdr_->bindDraw();
  oogl::viewport(fboHdr_);
  OOGL_CALL(glClearDepth(1.0f));
  OOGL_CALL(glClear(GL_DEPTH_BUFFER_BIT));

  renderObjects();


  //---------------------------------------------------------------------------
  //	PREP AUTO EXPOSURE
  //---------------------------------------------------------------------------
  //	render initial log luminance:
  fboLuma_->bindDraw();
  oogl::viewport(fboLuma_);
  shaderLuminance_->use();
  glActiveTexture(GL_TEXTURE0);
  texHdr_->bind();
  frm::modelSAQuad->enable(shaderLuminance_, this);
  frm::modelSAQuad->render();
  frm::modelSAQuad->disable();

  // progressive downsample into mip levels:
  shaderAvgMinMax_->use(); // renderToMipmap() assumes this call was made
  renderToMipmap(fboLuma_, shaderAvgMinMax_);

  //	adapt luminance value based on previous frame:
  fboAdaptLuma_[currentAdaptLuma_]->bind();
  oogl::viewport(fboAdaptLuma_[currentAdaptLuma_]);
  shaderLuminanceAdapt_->use();
  OOGL_CALL(glUniform1f(shaderLuminanceAdapt_->getUniformLocation("uAdaptionRate"), dt * adaptionRate_));
  glActiveTexture(GL_TEXTURE0);
  texLuma_->bind();
  glActiveTexture(GL_TEXTURE1);
  texAdaptLuma_[previousAdaptLuma]->bind();
  frm::modelSAQuad->enable(shaderLuminance_, this);
  frm::modelSAQuad->render();
  frm::modelSAQuad->disable();

  //---------------------------------------------------------------------------
  //	GENERATE/APPLY BLOOM
  //---------------------------------------------------------------------------
  //	downsample/threshold:
  fboAuxA_->bindDraw();
  oogl::viewport(fboAuxA_);
  shaderScaleBias_->use();
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uScale"), 1, sml::vec4f(bloomScale_)));
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uBias"), 1, sml::vec4f(bloomBias_)));
  OOGL_CALL(glActiveTexture(GL_TEXTURE0));
  texHdr_->bind();
  frm::modelSAQuad->enable(shaderScaleBias_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();

  //	gaussian blur:
  gaussBlur(fboAuxA_, fboAuxB_, fboAuxC_, (int)bloomBlurRadius_);

  //	upsample/blend:
  OOGL_CALL(glEnable(GL_BLEND));
  OOGL_CALL(glBlendFunc(GL_ONE, GL_ONE)); // additive
  fboHdr_->bindDraw();
  oogl::viewport(fboHdr_);
  shaderScaleBias_->use();
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uScale"), 1, sml::vec4f(1.0f)));
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uBias"), 1, sml::vec4f(0.0f)));
  texAuxC_->bind();
  frm::modelSAQuad->enable(shaderScaleBias_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();
  OOGL_CALL(glDisable(GL_BLEND));

  //---------------------------------------------------------------------------
  //	GENERATE LENS FLARE
  //---------------------------------------------------------------------------
    
  //	downsample/threshold:
  fboAuxA_->bindDraw();
  oogl::viewport(fboAuxA_);
  shaderScaleBias_->use();
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uScale"), 1, sml::vec4f(flareScale_)));
  OOGL_CALL(glUniform4fv(shaderScaleBias_->getUniformLocation("uBias"), 1, sml::vec4f(flareBias_)));
  OOGL_CALL(glActiveTexture(GL_TEXTURE0));
  texHdr_->bind();
  frm::modelSAQuad->enable(shaderScaleBias_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();

  // feature generation:
  fboAuxB_->bindDraw();
  shaderLensflare_->use();
  OOGL_CALL(glUniform1i(shaderLensflare_->getUniformLocation("uSamples"), (int)flareSamples_));
  OOGL_CALL(glUniform1f(shaderLensflare_->getUniformLocation("uDispersal"), flareDispersal_));
  OOGL_CALL(glUniform1f(shaderLensflare_->getUniformLocation("uHaloWidth"), flareHaloWidth_));
  OOGL_CALL(glUniform1f(shaderLensflare_->getUniformLocation("uDistortion"), flareDistortion_));
  OOGL_CALL(glActiveTexture(GL_TEXTURE0));
  texAuxA_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE1));
  texLensColor_->bind();
  frm::modelSAQuad->enable(shaderLensflare_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();

  //	gaussian blur:
  gaussBlur(fboAuxB_, fboAuxA_, fboAuxC_, (int)flareBlurRadius_);

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
  texHdr_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE1));
  texDepth_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE2));
  texAdaptLuma_[currentAdaptLuma_]->bind();

  OOGL_CALL(glActiveTexture(GL_TEXTURE3));
  texAuxC_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE4));
  texLensDirt_->bind();
  OOGL_CALL(glActiveTexture(GL_TEXTURE5));
  texLensStar_->bind();


  frm::modelSAQuad->enable(shaderPostProcess_, this);
  frm::modelSAQuad->render(GL_TRIANGLE_STRIP);
  frm::modelSAQuad->disable();

  prevViewProjMat = camera.getViewProjectionMatrix();

  return true;
}

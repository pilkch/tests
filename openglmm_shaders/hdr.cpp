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
#include "hdr.h"
#include "main.h"

// ** cHDR

cHDR::cHDR() :
  data(nullptr),

  fMaxRGBValue(1.0f)
{
}

void cHDR::Init(opengl::cContext& context)
{
  context.CreateShader(shaderPassThrough, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/passthrough2d.frag"));
  context.CreateShader(shaderLuminance, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/hdr/luminance.frag"));
  context.CreateShader(shaderMinification, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/hdr/minification.frag"));
  context.CreateShader(shaderToneMapping, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/hdr/tone_mapping.frag"));
  context.CreateShader(shaderBrightPixels, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/hdr/bright_pixels.frag"));
  context.CreateShader(shaderBlurH, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/hdr/blurh.frag"));
  context.CreateShader(shaderBlurV, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/hdr/blurv.frag"));


  context.BindShader(shaderBlurH);
  context.SetShaderConstant("Width", 5);
  context.UnBindShader(shaderBlurH);
  context.BindShader(shaderBlurV);
  context.SetShaderConstant("Width", 5);
  context.UnBindShader(shaderBlurV);

  data = new float[4096];
}

void cHDR::Destroy(opengl::cContext& context)
{
  delete[] data;

  for (size_t i = 0; i < 4; i++) context.DestroyStaticVertexBufferObject(bloomToScreenVBO[i]);

  context.DestroyTextureFrameBufferObject(fboLuminanceBuffer);
  for (size_t i = 0; i < 8; i++) {
    if (MinificationBuffer[i].fbo.IsValid()) context.DestroyTextureFrameBufferObject(MinificationBuffer[i].fbo);
    if (MinificationBuffer[i].vbo.IsCompiled()) context.DestroyStaticVertexBufferObject(MinificationBuffer[i].vbo);
  }
  context.DestroyTextureFrameBufferObject(fboBrightPixelsBuffer);
  for (size_t i = 0; i < 12; i++) {
    if (BloomBuffer[i].fbo.IsValid()) context.DestroyTextureFrameBufferObject(BloomBuffer[i].fbo);
    if (BloomBuffer[i].vbo.IsCompiled()) context.DestroyStaticVertexBufferObject(BloomBuffer[i].vbo);
  }

  context.DestroyShader(shaderPassThrough);
  context.DestroyShader(shaderLuminance);
  context.DestroyShader(shaderMinification);
  context.DestroyShader(shaderToneMapping);
  context.DestroyShader(shaderBrightPixels);
  context.DestroyShader(shaderBlurH);
  context.DestroyShader(shaderBlurV);
}

float cHDR::GetExposure() const
{
  return fMaxRGBValue;
}

opengl::cTextureFrameBufferObject& cHDR::GetBrightPixelsBuffer()
{
  return fboBrightPixelsBuffer;
}

void cHDR::Resize(cApplication& application, opengl::cContext& context)
{
  const size_t Width = context.GetWidth();
  const size_t Height = context.GetHeight();

  context.CreateTextureFrameBufferObject(fboLuminanceBuffer, Width, Height, opengl::PIXELFORMAT::R8G8B8A8); // Float 16 RGBA

  int i = 0, x = int(Width), y = int(Height);

  do {
    x /= 2;
    y /= 2;

    context.CreateTextureFrameBufferObject(MinificationBuffer[i].fbo, x, y, opengl::PIXELFORMAT::R8G8B8A8); // Float 16 RGBA

    i++;
  } while (x > 32 || y > 32);

  context.CreateTextureFrameBufferObject(fboBrightPixelsBuffer, Width, Height, opengl::PIXELFORMAT::R8G8B8A8); // RGBA8

  for (int i = 0; i < 4; i++) {
    const int ds = 2 * (i + 1);

    for (int ii = 0; ii < 3; ii++) {
      context.CreateTextureFrameBufferObject(BloomBuffer[(3 * i) + ii].fbo, Width / ds, Height / ds, opengl::PIXELFORMAT::R8G8B8A8); // RGBA8
    }
  }


  // Vertex buffer objects

  i = 0;
  int vboX = int(Width);
  int vboY = int(Height);
  int textureX = int(Width);
  int textureY = int(Height);

  // The first vbo is used for rendering the LuminanceBuffer
  application.CreateScreenRectVBO(MinificationBuffer[i].vbo, 1.0f, 1.0f, Width, Height);

  i = 1;

  do {
    vboX /= 2;
    vboY /= 2;

    application.CreateScreenRectVBO(MinificationBuffer[i].vbo, 1.0f, 1.0f, textureX, textureY);

    textureX /= 2;
    textureY /= 2;

    i++;
  } while ((vboX > 32) || (vboY > 32));


  for (size_t i = 0; i < 4; i++) {
    const size_t ds = 2 * (i + 1);

    // These are actually the VBOs to use for rendering from BrightPixelsBuffer -> BloomBuffer[0] -> BloomBuffer[1] -> BloomBuffer[2]
    const size_t index = (3 * i);

    // BrightPixelsBuffer -> BloomBuffer[0]
    //application.CreateScreenRectVBO(BloomBuffer[index].vbo, 1.0f, 1.0f, Width, Height);

    // BloomBuffer[0] -> BloomBuffer[1]
    application.CreateScreenRectVBO(BloomBuffer[index + 1].vbo, 1.0f, 1.0f, Width / ds, Height / ds);

    // BloomBuffer[1] -> BloomBuffer[2]
    application.CreateScreenRectVBO(BloomBuffer[index + 2].vbo, 1.0f, 1.0f, Width / ds, Height / ds);
  }

  // Create the final vertex buffer objects for rendering each bloom buffer to the screen
  // NOTE: We set them in reverse order as we actually want to use them from smallest to largest
  for (size_t i = 0; i < 4; i++) {
    const size_t ds = 2 * (i + 1);
    application.CreateScreenRectVBO(bloomToScreenVBO[3 - i], 1.0f, 1.0f, Width / ds, Height / ds);
  }
}

void cHDR::RenderBloom(cApplication& application, spitfire::durationms_t currentTime, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output)
{
  const size_t Width = context.GetWidth();
  const size_t Height = context.GetHeight();
  static spitfire::durationms_t lastDurationTime = currentTime;
  const float FrameTime = 0.0000001f; //(currentTime - lastDurationTime) * 0.001f;

  opengl::cTextureFrameBufferObject& HDRColorBuffer = input;

  // get the maximal value of the RGB components of the HDR image -----------------------------------------------------------

  static float maxrgbvalue = 1.0f, mrgbvi, oldmaxrgbvalue = maxrgbvalue;
  static spitfire::durationms_t LastTime = 0;
  static spitfire::durationms_t LastAdjustmentTime = 0;

  spitfire::durationms_t Time = GetTickCount();

  if (Time - LastTime > 125) { // 8 times per second only ----------------------------------------------------------------------
    LastTime = Time;

    // render LuminanceBuffer texture -------------------------------------------------------------------------------------

    {
      // Render the HDR texture to the luminance buffer
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      context.SetClearColour(clearColour);

      opengl::cTextureFrameBufferObject& frameBufferTo = fboLuminanceBuffer;
      context.BeginRenderToTexture(frameBufferTo);

      context.BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      application.RenderScreenRectangle(HDRColorBuffer, shaderLuminance);

      context.EndRenderMode2D();

      context.EndRenderToTexture(frameBufferTo);
    }

    int i = 0, x = int(Width), y = int(Height);
    float odx = 1.0f;
    float ody = 1.0f;

    // downscale LuminanceBuffer texture to less than 32x32 pixels by ping ponging back and forth from the luminance buffer and then between the minification buffers which decrease in size

    do {
      x /= 2;
      y /= 2;

      glViewport(0, 0, x, y);

      context.BeginRenderToTexture(MinificationBuffer[i].fbo);
      const opengl::cTexture& texture = (i == 0) ? fboLuminanceBuffer : MinificationBuffer[i - 1].fbo;
      context.BindTexture(0, texture);
      context.BindShader(shaderMinification);
      context.SetShaderConstant("odx", odx);
      context.SetShaderConstant("ody", ody);
      application.RenderScreenRectangleShaderAndTextureAlreadySet(MinificationBuffer[i].vbo);
      context.UnBindShader(shaderMinification);
      context.UnBindTexture(0, texture);
      context.EndRenderToTexture(MinificationBuffer[i].fbo);

      i++;
    } while (x > 32 || y > 32);

    glViewport(0, 0, int(Width), int(Height));

    // read downscaled LuminanceBuffer texture data -----------------------------------------------------------------------
    {
      const opengl::cTextureFrameBufferObject& downScaledLumincanceBuffer = MinificationBuffer[i - 1].fbo;
      const GLenum textureType = downScaledLumincanceBuffer.GetTextureType();
      glBindTexture(textureType, downScaledLumincanceBuffer.GetTexture());
      glGetTexImage(textureType, 0, GL_RGBA, GL_FLOAT, data);
      glBindTexture(textureType, 0);
    }

    // get the maximal luminance value ------------------------------------------------------------------------------------

    maxrgbvalue = 0.0f;

    for (int p = 0; p < x * y * 4; p += 4) maxrgbvalue = max(maxrgbvalue, data[p]);

    if (maxrgbvalue < 1.0) maxrgbvalue = 1.0;

    if (maxrgbvalue != oldmaxrgbvalue) mrgbvi = abs(maxrgbvalue - fMaxRGBValue);

    oldmaxrgbvalue = maxrgbvalue;
  }

  const float fLastMaxRGBValue = fMaxRGBValue;

  if (fMaxRGBValue < maxrgbvalue) {
    fMaxRGBValue += mrgbvi * FrameTime;
    if (fMaxRGBValue > maxrgbvalue) fMaxRGBValue = maxrgbvalue;
  }
  if (fMaxRGBValue > maxrgbvalue) {
    fMaxRGBValue -= mrgbvi * FrameTime;
    if (fMaxRGBValue < maxrgbvalue) fMaxRGBValue = maxrgbvalue;
  }

  if (Time - LastAdjustmentTime > 100) { // 10 times per second only ----------------------------------------------------------------------
    LastAdjustmentTime = Time;

    // Gently adjust the max RGB value until it matches our desired max RGB value
    fMaxRGBValue = fLastMaxRGBValue + (0.1f * (maxrgbvalue - fLastMaxRGBValue));
  }

  //LOG("fMaxRGBValue = ", fMaxRGBValue);

  // render BrightPixelsBuffer texture --------------------------------------------------------------------------------------

  context.BeginRenderToTexture(fboBrightPixelsBuffer);
  application.RenderScreenRectangle(HDRColorBuffer, shaderBrightPixels);
  context.EndRenderToTexture(fboBrightPixelsBuffer);

  // downscale and blur BrightPixelsBuffer texture 4x -----------------------------------------------------------------------

  for (int i = 0; i < 4; i++) {
    int ds = 2 * (i + 1);

    // set viewport to 1/ds of the screen ----------------------------------------------------------------------------------

    glViewport(0, 0, int(Width / ds), int(Height / ds));

    const size_t index = (i * 3);

    // downscale ----------------------------------------------------------------------------------------------------------

    context.BeginRenderToTexture(BloomBuffer[index].fbo);
    application.RenderScreenRectangle(fboBrightPixelsBuffer, shaderPassThrough);
    context.EndRenderToTexture(BloomBuffer[index].fbo);

    const float odw = 1.0f;
    const float odh = 1.0f;

    // horizontal blur ----------------------------------------------------------------------------------------------------

    context.BeginRenderToTexture(BloomBuffer[index + 1].fbo);
    context.BindTexture(0, BloomBuffer[index].fbo);
    context.BindShader(shaderBlurH);
    context.SetShaderConstant("odw", odw);
    application.RenderScreenRectangleShaderAndTextureAlreadySet(BloomBuffer[index + 1].vbo);
    context.UnBindShader(shaderBlurH);
    context.BindTexture(0, BloomBuffer[index].fbo);
    context.EndRenderToTexture(BloomBuffer[index + 1].fbo);

    // vertical blur ------------------------------------------------------------------------------------------------------

    context.BeginRenderToTexture(BloomBuffer[index + 2].fbo);
    context.BindTexture(0, BloomBuffer[index + 1].fbo);
    context.BindShader(shaderBlurV);
    context.SetShaderConstant("odh", odh);
    application.RenderScreenRectangleShaderAndTextureAlreadySet(BloomBuffer[index + 2].vbo);
    context.UnBindShader(shaderBlurV);
    context.UnBindTexture(0, BloomBuffer[index + 1].fbo);
    context.EndRenderToTexture(BloomBuffer[index + 2].fbo);
  }

  // Combine the HDRColorBuffer texture and bright pixels buffers into a single texture
  {
    // Render the HDRColorBuffer to our framebuffer
    const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
    context.SetClearColour(clearColour);

    opengl::cTextureFrameBufferObject& frameBuffer = output;
    context.BeginRenderToTexture(frameBuffer);

    // Now draw an overlay of our rendered textures
    context.BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

    application.RenderScreenRectangle(HDRColorBuffer, shaderPassThrough);

    // blend 4 downscaled and blurred BloomBuffer textures over the screen ---------------------------------------------

    glEnable(GL_BLEND);

    #if 0
    // Original from John Chapman (Sky is too bright)
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

    for (int i = 0; i < 4; i++) {
      const size_t index = ((3 - i) * 3) + 2;
      application.RenderScreenRectangle(BloomBuffer[index].fbo, *pShaderPassThrough, bloomToScreenVBO[i]);
    }
    #elif 1
    // Tweaked version (Sky is darker but still bright)
    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

    for (int i = 0; i < 4; i++) {
      const size_t index = ((3 - i) * 3) + 2;
      application.RenderScreenRectangle(BloomBuffer[index].fbo, shaderPassThrough, bloomToScreenVBO[i]);
    }
    #elif 0
    // Tweaked verison (Sky is blue and bloom is not as pronounced)
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    const size_t i = 0;
    const size_t index = ((3 - i) * 3) + 2;
    application.RenderScreenRectangle(BloomBuffer[index].fbo, *pShaderPassThrough, bloomToScreenVBO[i]);
    #endif

    glDisable(GL_BLEND);

    context.EndRenderMode2D();

    context.EndRenderToTexture(frameBuffer);
  }
}

void cHDR::RenderToneMapping(cApplication& application, spitfire::durationms_t currentTime, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output)
{
  // Render HDR texture (After the application has applied further processing) to LDR texture with tone mapping shader applied

  context.BeginRenderToTexture(output);
  context.BindShader(shaderToneMapping);
  context.BindTexture(0, input);
  context.SetShaderConstant("fMaxRGBValue", fMaxRGBValue);
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
  context.UnBindTexture(0, input);
  context.UnBindShader(shaderToneMapping);
  context.EndRenderToTexture(output);
}

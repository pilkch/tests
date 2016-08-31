#include <cassert>
#include <cmath>

#include <string>

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/util/log.h>

// Application headers
#include "anamorphic_lens_flare.h"
#include "main.h"

// ** cAnamorphicLensFlare

const size_t horizontalDownScale0 = 2;
const size_t horizontalDownScale1 = 4;
const size_t horizontalDownScale2 = 8;
const size_t horizontalDownScale3 = 16;
const size_t horizontalDownScale4 = 32;
const size_t horizontalDownScale5 = 32;
const size_t verticalDownScale = 4;

void cAnamorphicLensFlare::Init(cApplication& application, opengl::cContext& context)
{
  context.CreateShader(shaderHorizontalBlur, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/anamorphiclensflare/horizontalblur.frag"));
  assert(shaderHorizontalBlur.IsCompiledProgram());
  
  context.CreateTextureFrameBufferObject(brightPixelsStretchedHorizontally0, context.GetWidth() / horizontalDownScale0, context.GetHeight(), opengl::PIXELFORMAT::R8G8B8A8);
  context.CreateTextureFrameBufferObject(brightPixelsStretchedHorizontally1, context.GetWidth() / horizontalDownScale1, context.GetHeight(), opengl::PIXELFORMAT::R8G8B8A8);
  context.CreateTextureFrameBufferObject(brightPixelsStretchedHorizontally2, context.GetWidth() / horizontalDownScale2, context.GetHeight(), opengl::PIXELFORMAT::R8G8B8A8);
  context.CreateTextureFrameBufferObject(brightPixelsStretchedHorizontally3, context.GetWidth() / horizontalDownScale3, context.GetHeight(), opengl::PIXELFORMAT::R8G8B8A8);
  context.CreateTextureFrameBufferObject(brightPixelsStretchedHorizontally4, context.GetWidth() / horizontalDownScale4, context.GetHeight(), opengl::PIXELFORMAT::R8G8B8A8);
  context.CreateTextureFrameBufferObject(brightPixelsStretchedHorizontally5, context.GetWidth() / horizontalDownScale5, context.GetHeight() / verticalDownScale, opengl::PIXELFORMAT::R8G8B8A8);
  
  context.CreateShader(shaderAnamorphicLensBlueTinge, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/anamorphiclensflare/bluetinge.frag"));
  assert(shaderAnamorphicLensBlueTinge.IsCompiledProgram());
}

void cAnamorphicLensFlare::Destroy(opengl::cContext& context)
{
  if (shaderHorizontalBlur.IsCompiledProgram()) context.DestroyShader(shaderHorizontalBlur);

  if (brightPixelsStretchedHorizontally0.IsValid()) context.DestroyTextureFrameBufferObject(brightPixelsStretchedHorizontally0);
  if (brightPixelsStretchedHorizontally1.IsValid()) context.DestroyTextureFrameBufferObject(brightPixelsStretchedHorizontally1);
  if (brightPixelsStretchedHorizontally2.IsValid()) context.DestroyTextureFrameBufferObject(brightPixelsStretchedHorizontally2);
  if (brightPixelsStretchedHorizontally3.IsValid()) context.DestroyTextureFrameBufferObject(brightPixelsStretchedHorizontally3);
  if (brightPixelsStretchedHorizontally4.IsValid()) context.DestroyTextureFrameBufferObject(brightPixelsStretchedHorizontally4);
  if (brightPixelsStretchedHorizontally5.IsValid()) context.DestroyTextureFrameBufferObject(brightPixelsStretchedHorizontally5);

  if (shaderAnamorphicLensBlueTinge.IsCompiledProgram()) context.DestroyShader(shaderAnamorphicLensBlueTinge);
}

void cAnamorphicLensFlare::Resize(opengl::cContext& context)
{
}

void cAnamorphicLensFlare::RenderInputBrightPixelsBlurred(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output)
{
  opengl::cStaticVertexBufferObject vbo;
  application.CreateScreenRectVBO(vbo, 1.0f, 1.0f, input.GetWidth(), input.GetHeight());

  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(output);

  context.BindTexture(0, input);

  context.BindShader(shaderHorizontalBlur);

  //application.RenderScreenRectangleShaderAndTextureAlreadySet();
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vbo);

  context.UnBindShader(shaderHorizontalBlur);

  context.UnBindTexture(0, input);

  context.EndRenderToTexture(output);

  context.DestroyStaticVertexBufferObject(vbo);
}

void cAnamorphicLensFlare::RenderBlurredToWithBlurTingeToOutput(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output)
{
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  opengl::cTextureFrameBufferObject& frameBufferFrom = input;
  opengl::cTextureFrameBufferObject& frameBufferTo = output;

  context.BeginRenderToTexture(frameBufferTo);

  // Render the scene and our bright pixels buffer with a blue tinge over the top
  context.BindTexture(0, frameBufferFrom);
  context.BindTexture(1, brightPixelsStretchedHorizontally5);

  context.BindShader(shaderAnamorphicLensBlueTinge);

  // Set our shader constants
  context.SetShaderConstant("brightPixelsStretchedScale", spitfire::math::cVec2(1.0f / float(horizontalDownScale4), 1.0f / float(verticalDownScale)));

  application.RenderScreenRectangleShaderAndTextureAlreadySet();

  context.UnBindShader(shaderAnamorphicLensBlueTinge);

  context.UnBindTexture(1, brightPixelsStretchedHorizontally5);
  context.UnBindTexture(0, frameBufferFrom);

  context.EndRenderToTexture(frameBufferTo);
}

void cAnamorphicLensFlare::Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& inputBrightPixels, opengl::cTextureFrameBufferObject& output)
{
  // Render a blurred image of the bright pixels in the scene
  RenderInputBrightPixelsBlurred(application, context, inputBrightPixels, brightPixelsStretchedHorizontally0);
  RenderInputBrightPixelsBlurred(application, context, brightPixelsStretchedHorizontally0, brightPixelsStretchedHorizontally1);
  RenderInputBrightPixelsBlurred(application, context, brightPixelsStretchedHorizontally1, brightPixelsStretchedHorizontally2);
  RenderInputBrightPixelsBlurred(application, context, brightPixelsStretchedHorizontally2, brightPixelsStretchedHorizontally3);
  RenderInputBrightPixelsBlurred(application, context, brightPixelsStretchedHorizontally3, brightPixelsStretchedHorizontally4);
  RenderInputBrightPixelsBlurred(application, context, brightPixelsStretchedHorizontally4, brightPixelsStretchedHorizontally5);

  // Render the scene with an overlayed anamorphic lens effect over the top
  RenderBlurredToWithBlurTingeToOutput(application, context, input, output);
}

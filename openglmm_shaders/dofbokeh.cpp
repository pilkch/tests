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
#include "dofbokeh.h"
#include "main.h"

// ** cDOFBokeh

cDOFBokeh::cDOFBokeh() :
  bDebugDOFShowFocus(false),
  fFocalLengthmm(18.0f),
  fFStop(3.6f),
  pShaderScreenRectDOFBokeh(nullptr)
{
}

float cDOFBokeh::GetFocalLengthmm() const
{
  return fFocalLengthmm;
}

float cDOFBokeh::GetFStop() const
{
  return fFStop;
}

void cDOFBokeh::SetDebugDOFShowFocus(bool bShow)
{
  bDebugDOFShowFocus = bShow;
}

bool cDOFBokeh::IsDebugDOFShowFocus() const
{
  return bDebugDOFShowFocus;
}

const float fFocalLengthDelta = 2.0f;

void cDOFBokeh::DecreaseFocalLength()
{
  fFocalLengthmm = max(10.0f, fFocalLengthmm - fFocalLengthDelta);
}

void cDOFBokeh::IncreaseFocalLength()
{
  fFocalLengthmm = min(250.0f, fFocalLengthmm + fFocalLengthDelta);
}

const float fFStopDelta = 0.1f;

void cDOFBokeh::DecreaseFStop()
{
  fFStop = max(0.1f, fFStop - fFStopDelta);
}

void cDOFBokeh::IncreaseFStop()
{
  fFStop = min(30.0f, fFStop + fFStopDelta);
}

void cDOFBokeh::Init(opengl::cContext& context)
{
  pShaderScreenRectDOFBokeh = context.CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/dofbokeh.frag"));
  assert(pShaderScreenRectDOFBokeh != nullptr);
}

void cDOFBokeh::Destroy(opengl::cContext& context)
{
  if (pShaderScreenRectDOFBokeh != nullptr) {
    context.DestroyShader(pShaderScreenRectDOFBokeh);
    pShaderScreenRectDOFBokeh = nullptr;
  }
}

void cDOFBokeh::Resize(cApplication& application, opengl::cContext& context)
{
  const opengl::cResolution resolution = application.GetResolution();

  context.BindShader(*pShaderScreenRectDOFBokeh);

  // Set our shader constants
  context.SetShaderConstant("textureSize", spitfire::math::cVec2(float(resolution.width), float(resolution.height)));

  context.UnBindShader(*pShaderScreenRectDOFBokeh);
}

void cDOFBokeh::Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& textureScreenDepth, opengl::cTextureFrameBufferObject& output)
{
  const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  opengl::cTextureFrameBufferObject& frameBufferTo = output;
  opengl::cTextureFrameBufferObject& frameBufferFrom = input;

  context.BeginRenderToTexture(frameBufferTo);

  context.BindTexture(0, frameBufferFrom);
  context.BindTextureDepthBuffer(1, textureScreenDepth);

  context.BindShader(*pShaderScreenRectDOFBokeh);

  context.SetShaderConstant("focalLength", fFocalLengthmm); //focal length in mm
  context.SetShaderConstant("fstop", fFStop); //f-stop value
  context.SetShaderConstant("showFocus", bDebugDOFShowFocus); // Show debug focus point and focal range (red = focal point, green = focal range)

  // Not required when using autofocus
  //context.SetShaderConstant("focalDepth", fFocalDepth);

  application.RenderScreenRectangleShaderAndTextureAlreadySet();

  context.UnBindShader(*pShaderScreenRectDOFBokeh);

  context.UnBindTextureDepthBuffer(1, textureScreenDepth);
  context.UnBindTexture(0, frameBufferFrom);

  context.EndRenderToTexture(frameBufferTo);
}

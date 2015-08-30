#ifndef HDR_H
#define HDR_H

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
#include "util.h"

class cApplication;

class cHDR
{
public:
  void Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(cApplication& application, opengl::cContext& context);

  void Render(cApplication& application, spitfire::durationms_t currentTime, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output);

  opengl::cTextureFrameBufferObject* LuminanceBuffer;
  cTextureFrameBufferObjectVBOPair MinificationBuffer[8];
  opengl::cTextureFrameBufferObject* LDRColorBuffer;
  opengl::cTextureFrameBufferObject* BrightPixelsBuffer;
  cTextureFrameBufferObjectVBOPair BloomBuffer[12];

  opengl::cShader* pShaderPassThrough;
  opengl::cShader* Luminance;
  opengl::cShader* Minification;
  opengl::cShader* ToneMapping;
  opengl::cShader* BrightPixels;
  opengl::cShader* BlurH;
  opengl::cShader* BlurV;

  opengl::cStaticVertexBufferObject bloomToScreenVBO[4];

  float *data;

  float Intensity;
};

#endif // HDR_H

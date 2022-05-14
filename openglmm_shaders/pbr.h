#pragma once

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
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Application headers
#include "util.h"

class cApplication;

class cPBR
{
public:
  bool Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(cApplication& application, opengl::cContext& context);

  // This is a shader that a PBR object can use for rendering in combination with the irradiance map, prefilter map and BRDF LUT texture below
  opengl::cShader& GetShader() { return shader; }

  opengl::cTexture& DebugGetTextureHDREquirectangularSpheremap() { return fboTextureHDREquirectangularSpheremap; }
  opengl::cTextureFrameBufferObject& DebugGetEnvCubemap() { return fboEnvCubemap; }

  opengl::cTextureFrameBufferObject& GetIrradianceMap() { return fboIrradianceMap; }
  opengl::cTextureFrameBufferObject& GetPrefilterMap() { return fboPrefilterMap; }
  opengl::cTextureFrameBufferObject& GetBRDFLUTTexture() { return fboBRDFLUTTexture; }

private:
  bool LoadAndRenderEquirectangularToCubemap(opengl::cContext& context);

  // After the other textures below have been created we don't need these two textures, we only hold onto them for debug purposes
  opengl::cTexture fboTextureHDREquirectangularSpheremap;
  opengl::cTextureFrameBufferObject fboEnvCubemap;

  opengl::cTextureFrameBufferObject fboIrradianceMap;
  opengl::cTextureFrameBufferObject fboPrefilterMap;
  opengl::cTextureFrameBufferObject fboBRDFLUTTexture;

  opengl::cShader shader;
};

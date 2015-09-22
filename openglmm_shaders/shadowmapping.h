#ifndef SHADOWMAPPING_H
#define SHADOWMAPPING_H

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

class cShadowMapping
{
public:
  cShadowMapping();

  void Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void BeginRenderToShadowMap(opengl::cContext& context);
  void EndRenderToShadowMap(opengl::cContext& context);

  void RenderObjectToShadowMapSetMatrices(opengl::cContext& context, const spitfire::math::cVec3& lightPosition, const spitfire::math::cVec3& lightDirection, const spitfire::math::cVec3& objectPosition, const spitfire::math::cQuaternion& objectRotation);

  opengl::cTextureFrameBufferObject& GetShadowMapTexture();
  opengl::cShader& GetRenderToShadowMapShader();
  opengl::cShader& GetShadowMapShader();

  spitfire::math::cMat4 GetDepthMVP() const;
  spitfire::math::cMat4 GetDepthBiasMVP() const;
  spitfire::math::cVec3 GetLightInvDirectionWorldSpace() const;

private:
  opengl::cTextureFrameBufferObject* pTextureDepthTexture;
  opengl::cShader* pShaderRenderToDepthTexture;
  opengl::cShader* pShaderShadowMap;
};

#endif // SHADOWMAPPING_H

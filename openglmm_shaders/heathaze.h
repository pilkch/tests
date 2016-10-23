#ifndef HEATHAZE_H
#define HEATHAZE_H

#include <list>

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/math/cMat4.h>

// libopenglmm headers
#include <libopenglmm/cShader.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>

// Application headers
#include "util.h"
#include "gaussian_blur.h"

class cApplication;

class cHeatHaze {
public:
  void Init(cApplication& application, opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void ReloadShaders(opengl::cContext& context);

  opengl::cTextureFrameBufferObject& GetHeatMapTexture() { return fboHeatMap; }

  void AddColdObject(const spitfire::math::cMat4& matModel, opengl::cStaticVertexBufferObject* pVBO);

  void BeginRender(cApplication& application, opengl::cContext& context, const spitfire::math::cMat4& matProjection, const spitfire::math::cMat4& matView);
  void EndRender(cApplication& application, opengl::cContext& context, spitfire::durationms_t time, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output);

private:
  std::list<std::pair<spitfire::math::cMat4, opengl::cStaticVertexBufferObject*>> lColdObjects;

  opengl::cShader shaderBlack; // For rendering objects that are not affected by heat haze
  opengl::cShader shaderHeatHighlights; // For rendering hot objects
  opengl::cShader shaderHeatHazeScreen; // For rendering all objects to the screen

  opengl::cTexture textureNoiseTiled;

  opengl::cTextureFrameBufferObject fboHeatMap;
};

#endif // HEATHAZE_H

#ifndef TRONGLOW_H
#define TRONGLOW_H

#include <list>

// Spitfire headers
#include <spitfire/math/cMat4.h>

// libopenglmm headers
#include <libopenglmm/cShader.h>
#include <libopenglmm/cVertexBufferObject.h>

// Application headers
#include "util.h"

class cApplication;

class cTronGlow {
public:
  void Init(cApplication& application, opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(cApplication& application, opengl::cContext& context);

  void AddNonGlowingObject(const spitfire::math::cMat4& matModel, opengl::cStaticVertexBufferObject* pVBO);

  void BeginRender(cApplication& application, opengl::cContext& context, const spitfire::math::cMat4& matProjection, const spitfire::math::cMat4& matView, opengl::cTextureFrameBufferObject& temp);
  void EndRender(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& temp, opengl::cTextureFrameBufferObject& output);

private:
  void RenderSceneWithTronGlow(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& temp, opengl::cTextureFrameBufferObject& output);

  opengl::cShader shaderBlack; // For rendering objects that are not glowing
  opengl::cShader shaderGlowHighlights; // For rendering objects that should glow
  opengl::cShader shaderTronGlowScreen; // For rendering all objects to the screen

  opengl::cStaticVertexBufferObject vbo;

  std::list<std::pair<spitfire::math::cMat4, opengl::cStaticVertexBufferObject*>> lNotGlowingObjects;
};

#endif // TRONGLOW_H

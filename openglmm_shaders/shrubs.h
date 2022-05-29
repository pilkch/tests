#pragma once

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/util/timer.h>

// libopenglmm headers
#include <libopenglmm/cShader.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>

class cApplication;

class cShrubs
{
public:
  bool Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void Resize(cApplication& application, opengl::cContext& context);

  opengl::cTexture& GetTexture() { return texture; }
  opengl::cShader& GetShader() { return shader; }
  opengl::cStaticVertexBufferObject& GetVBO() { return vbo; }

private:
  opengl::cTexture texture;
  opengl::cShader shader;
  opengl::cStaticVertexBufferObject vbo;
};

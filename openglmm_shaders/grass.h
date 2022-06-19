#pragma once

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/util/timer.h>
#include <spitfire/math/geometry.h>

// libopenglmm headers
#include <libopenglmm/cShader.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>

// Breathe headers
#include <breathe/physics/verlet.h>

class cGrass
{
public:
  bool Init(opengl::cContext& context, breathe::physics::verlet::cWorld& physicsWorld);
  void Destroy(opengl::cContext& context);

  void Update(opengl::cContext& context, const spitfire::math::cVec3& cameraPosition, breathe::physics::verlet::cWorld& physicsWorld);

  spitfire::math::cAABB3 GetBoundingBox() const;

  opengl::cTexture& GetTexture() { return texture; }
  opengl::cShader& GetShader() { return shader; }
  opengl::cStaticVertexBufferObject& GetVBO() { return vbo; }

private:
  // NOTE: We could use a grid of say 4x4 physics groups and divide the grass blades up into them
  breathe::physics::verlet::cGroup physicsGroup;

  opengl::cTexture texture;
  opengl::cShader shader;
  opengl::cStaticVertexBufferObject vbo;
};

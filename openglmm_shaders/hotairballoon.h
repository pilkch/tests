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

class cApplication;

class cHotAirBalloon
{
public:
  cHotAirBalloon();

  bool Init(cApplication& application, opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void IncreaseLift();
  void DecreaseLift();

  void Update(opengl::cContext& context);

  spitfire::math::cSphere GetBoundingSphere() const;

  opengl::cTexture& GetBalloonTexture() { return textureBalloon; }
  opengl::cTexture& GetBasketTexture() { return textureBasket; }
  opengl::cTexture& GetRopeTexture() { return textureRope; }
  opengl::cStaticVertexBufferObject& GetBalloonVBO() { return (bDebug ? vboBalloonDebug : vboBalloon); }
  opengl::cStaticVertexBufferObject& GetBasketVBO() { return vboBasket; }
  opengl::cStaticVertexBufferObject& GetRopeVBO() { return vboRope; }

  const spitfire::math::cVec3& GetBalloonCentrePoint() const { return balloonCentrePoint; }
  const spitfire::math::cMat4& GetBalloonRotation() const { return balloonRotation; }

  bool IsDebug() const { return bDebug; }
  void ToggleDebug() { bDebug = !bDebug; }

private:
  void CreateBox(const spitfire::math::cVec3& center, float fHalfWidth, float fHalfHeight, float fBoxStiffness);
  void CreateRope(breathe::physics::verlet::Particle& from, breathe::physics::verlet::Particle& to);

  // We provide our own physics world because the wind for the flags is a bit strong and blows our hot air balloon away
  breathe::physics::verlet::cWorld physicsWorld;

  breathe::physics::verlet::cGroup physicsGroup;

  opengl::cTexture textureBalloon;
  opengl::cTexture textureBasket;
  opengl::cTexture textureRope;
  opengl::cStaticVertexBufferObject vboBalloon;
  opengl::cStaticVertexBufferObject vboBalloonDebug;
  opengl::cStaticVertexBufferObject vboBasket;
  opengl::cStaticVertexBufferObject vboRope;

  float fBalloonLift;

  spitfire::math::cVec3 basketCentrePoint;
  spitfire::math::cVec3 balloonCentrePoint;
  spitfire::math::cMat4 balloonRotation;

  bool bDebug;
};

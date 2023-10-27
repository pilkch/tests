#include <cassert>
#include <cmath>

#include <array>
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

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>

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
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Application headers
#include "hotairballoon.h"
#include "main.h"

namespace util {

void AddQuad(opengl::cGeometryBuilder_v3_n3_t2& builder, const spitfire::math::cVec3& p0, const spitfire::math::cVec3& p1, const spitfire::math::cVec3& p2, const spitfire::math::cVec3& p3, const spitfire::math::cVec2& t0, const spitfire::math::cVec2& t1, const spitfire::math::cVec2& t2, const spitfire::math::cVec2& t3, const spitfire::math::cVec3& normal)
{
  builder.PushBack(p0, normal, t0);
  builder.PushBack(p1, normal, t1);
  builder.PushBack(p2, normal, t2);
  builder.PushBack(p3, normal, t3);
  builder.PushBack(p2, normal, t2);
  builder.PushBack(p1, normal, t1);
}

}

// Physics (Added to the physics world in this order):
//  A reinforced cube for the basket
//  A reinforced cube for the balloon part
//  Ropes connecting the balloon to the basket
//  Rope attaching the basket to the ground
//
// To get the hot air balloon to float bouyancy is enforced on the 8 particles of the cube, this basically just means we push the particles of the cube up each physics step
//
//
// Rendering:
//  We get the centre point of the 8 particles of the balloon cube, find the midpoint, then use furthest away particle as the radius of the balloon
//  We can create a cube for the basket from the 8 basket physics particles
//  Rope is created from the physics particles
//

namespace {

const float fGravity = -9.8f;

const float fBalloonRadius = 2.0f;
const float fAttachmentRopeLength = 2.0f;
const float fBasketHalfWidth = 1.0f;

const spitfire::math::cVec3 attachmentPointOnGround(30.0f, 0.01f, 0.0f);
const spitfire::math::cVec3 attachmentPointOnBasket(attachmentPointOnGround + spitfire::math::cVec3(0.0f, fAttachmentRopeLength, 0.0f));
const spitfire::math::cVec3 basketCentrePoint(attachmentPointOnBasket + spitfire::math::cVec3(fBasketHalfWidth, fBasketHalfWidth, 0.0f));

const float fBasketStiffness = 0.99f;
const float fBalloonStiffness = 0.8f;
const float fRopeStiffness = 0.8f;

// The number of rope segments per rope
const size_t nRopeSegments = 10;

const float DEFAULT_BALLOON_LIFT = 0.0051f;
const float BALLOON_LIFT_CHANGE = 0.001f;

}

cHotAirBalloon::cHotAirBalloon() :
  fBalloonLift(DEFAULT_BALLOON_LIFT),
  balloonCentrePoint(basketCentrePoint + spitfire::math::cVec3(0.0f, fBasketHalfWidth + 2.0f + fBalloonRadius, 0.0f)),
  bDebug(false)
{
}

void cHotAirBalloon::IncreaseLift()
{
  fBalloonLift += BALLOON_LIFT_CHANGE;
}

void cHotAirBalloon::DecreaseLift()
{
  fBalloonLift -= BALLOON_LIFT_CHANGE;

  if (fBalloonLift < 0.0f) fBalloonLift = 0.0f;
}

spitfire::math::cSphere cHotAirBalloon::GetBoundingSphere() const
{
  const spitfire::math::cVec3 lower(basketCentrePoint - spitfire::math::cVec3(0.0f, fBasketHalfWidth, 0.0f));
  const spitfire::math::cVec3 upper(balloonCentrePoint + spitfire::math::cVec3(0.0f, fBalloonRadius, 0.0f));
  const float fTotalHeight = upper.y - lower.y;

  spitfire::math::cSphere boundingSphere;
  boundingSphere.position = 0.5f * (upper + lower);
  boundingSphere.fRadius = 0.5f * fTotalHeight;
  return boundingSphere;
}

void cHotAirBalloon::CreateBox(const spitfire::math::cVec3& centre, float fHalfWidth, float fHalfHeight, float fBoxStiffness)
{
  const size_t particle_offset = physicsGroup.particles.size();

  // Create the particles
  const spitfire::math::cVec3 corners[8] = {
    spitfire::math::cVec3(-fHalfWidth, -fHalfHeight, -fHalfWidth),
    spitfire::math::cVec3(-fHalfWidth, -fHalfHeight, fHalfWidth),
    spitfire::math::cVec3(fHalfWidth, -fHalfHeight, fHalfWidth),
    spitfire::math::cVec3(fHalfWidth, -fHalfHeight, -fHalfWidth),
    spitfire::math::cVec3(-fHalfWidth, fHalfHeight, -fHalfWidth),
    spitfire::math::cVec3(-fHalfWidth, fHalfHeight, fHalfWidth),
    spitfire::math::cVec3(fHalfWidth, fHalfHeight, fHalfWidth),
    spitfire::math::cVec3(fHalfWidth, fHalfHeight, -fHalfWidth),
  };
  for (const auto& c : corners) {
    const spitfire::math::cVec3 point = centre + c;
    breathe::physics::verlet::Particle p(point);
    physicsGroup.particles.push_back(p);
  }

  // Create the top and bottom squares
  for (size_t i = 0; i < 2; i++) {
    const size_t points_square_offset = particle_offset + (i * 4);
    std::cout<<"Adding springs for layer "<<points_square_offset<<std::endl;
    breathe::physics::verlet::Particle* a = &physicsGroup.particles[points_square_offset];
    breathe::physics::verlet::Particle* b = &physicsGroup.particles[points_square_offset + 1];
    breathe::physics::verlet::Particle* c = &physicsGroup.particles[points_square_offset + 2];
    breathe::physics::verlet::Particle* d = &physicsGroup.particles[points_square_offset + 3];

    const float ab = (a->pos - b->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, b, ab, fBoxStiffness));
    const float bc = (b->pos - c->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(b, c, bc, fBoxStiffness));
    const float cd = (c->pos - d->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(c, d, cd, fBoxStiffness));
    const float da = (d->pos - a->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(d, a, da, fBoxStiffness));
    std::cout<<"Length of spring "<<ab<<", "<<bc<<", "<<cd<<", "<<da<<", "<<std::endl;
  }

  // Link each particle to the particle above it to make a cube
  {
    std::cout<<"Adding springs between layers"<<std::endl;

    for (size_t i = 0; i < 4; i++) {
      const size_t points_square_offset = particle_offset + i;
      breathe::physics::verlet::Particle* a = &physicsGroup.particles[points_square_offset];
      breathe::physics::verlet::Particle* b = &physicsGroup.particles[points_square_offset + 4];

      const float ab = (a->pos - b->pos).GetLength();
      std::cout<<"Length of spring "<<ab<<std::endl;
      physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, b, ab, fBoxStiffness));
    }
  }

  // Link each particle to the particle diagonally opposite on the square above it
  {
    std::cout<<"Adding diagonal springs between layers"<<std::endl;
    const size_t points_square_offset = particle_offset;

    breathe::physics::verlet::Particle* a = &physicsGroup.particles[points_square_offset];
    breathe::physics::verlet::Particle* b = &physicsGroup.particles[points_square_offset + 1];
    breathe::physics::verlet::Particle* c = &physicsGroup.particles[points_square_offset + 2];
    breathe::physics::verlet::Particle* d = &physicsGroup.particles[points_square_offset + 3];
    breathe::physics::verlet::Particle* e = &physicsGroup.particles[points_square_offset + 4];
    breathe::physics::verlet::Particle* f = &physicsGroup.particles[points_square_offset + 5];
    breathe::physics::verlet::Particle* g = &physicsGroup.particles[points_square_offset + 6];
    breathe::physics::verlet::Particle* h = &physicsGroup.particles[points_square_offset + 7];

    const float ag = (a->pos - g->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, g, ag, fBoxStiffness));
    const float bh = (b->pos - h->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(b, h, bh, fBoxStiffness));
    const float ce = (c->pos - e->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(c, e, ce, fBoxStiffness));
    const float df = (d->pos - f->pos).GetLength();
    physicsGroup.springs.push_back(breathe::physics::verlet::Spring(d, f, df, fBoxStiffness));
    std::cout<<"Length of spring "<<ag<<", "<<bh<<", "<<ce<<", "<<df<<", "<<std::endl;
  }
}

void cHotAirBalloon::CreateRope(breathe::physics::verlet::Particle& from, breathe::physics::verlet::Particle& to)
{
  // List of particles that will form our rope, including the from and to particles
  std::vector<breathe::physics::verlet::Particle*> particlesToConnect;

  // Add our first point
  particlesToConnect.push_back(&from);

  // Create the rope particles between the from and to points
  spitfire::math::cVec3 point = from.pos;
  const spitfire::math::cVec3 ropeDirection = (to.pos - from.pos).GetNormalised();
  const float fRopeSegmentLength = (to.pos - from.pos).GetLength() / float(nRopeSegments);
  for (size_t i = 0; i < nRopeSegments - 1; i++) {
    point += fRopeSegmentLength * ropeDirection;
    breathe::physics::verlet::Particle p0(point);
    physicsGroup.particles.push_back(p0);

    // Add the rope particle to the list of particles in the rope
    particlesToConnect.push_back(&physicsGroup.particles.back());
  }

  // And add the to point
  particlesToConnect.push_back(&to);


  // Now link all the particles together with rope joints
  const size_t n = (particlesToConnect.size() - 1);
  for (size_t i = 0; i < n; i++) {
    breathe::physics::verlet::Particle* a = particlesToConnect[i];
    breathe::physics::verlet::Particle* b = particlesToConnect[i + 1];

    const float ab = (a->pos - b->pos).GetLength();
    physicsGroup.ropes.push_back(breathe::physics::verlet::Rope(a, b, ab, fRopeStiffness));
  }
}

bool cHotAirBalloon::Init(cApplication& application, opengl::cContext& context)
{
  context.CreateTexture(textureBalloon, TEXT("textures/hot_air_balloon.png"));
  if (textureBalloon.GetPixelFormat() != voodoo::PIXELFORMAT::R8G8B8) {
    return false;
  }

  context.CreateTexture(textureBasket, TEXT("textures/basket.png"));
  if (textureBasket.GetPixelFormat() != voodoo::PIXELFORMAT::R8G8B8) {
    return false;
  }

  context.CreateTexture(textureRope, TEXT("textures/rope.png"));
  if (textureRope.GetPixelFormat() != voodoo::PIXELFORMAT::R8G8B8) {
    return false;
  }



  const float fBalloonScale = 5.0f;
  const spitfire::math::cVec3 balloonTranlsation(0.0f, -2.0f, 0.0f);

  if (!application.CreateVBOFromObjFile(vboBalloon, TEXT("models/hot_air_balloon.obj"), fBalloonScale, balloonTranlsation)) {
    return false;
  }

  // Set the debug balloon VBO to something
  if (!application.CreateVBOFromObjFile(vboBalloonDebug, TEXT("models/hot_air_balloon.obj"), fBalloonScale, balloonTranlsation)) {
    return false;
  }

  // No wind
  breathe::physics::verlet::cWindProperties windProperties;

  physicsWorld.Init(fGravity, windProperties);


  // TODO: This is an ugly hack, we need to allocate these up front so that they don't get reallocated and the spring pointers don't get invalidated
  // Particles for 2 boxes, 5 ropes
  const size_t nParticles = (2 * 4) + ((1 + 4) * (nRopeSegments + 1));
  // Springs for 2 boxes
  const size_t nSprings = 2 * 16;
  // Ropes for 5 ropes
  const size_t nRopes = (1 + 4) * nRopeSegments;
  physicsGroup.particles.reserve(nParticles);
  physicsGroup.springs.reserve(nSprings);
  physicsGroup.ropes.reserve(nRopes);

  // Create the basket
  CreateBox(basketCentrePoint, fBasketHalfWidth, fBasketHalfWidth, fBasketStiffness);

  // Create the balloon which is just another box
  CreateBox(balloonCentrePoint, fBalloonRadius, fBalloonRadius, fBalloonStiffness);


  // Link the basket and the balloon together with ropes
  std::cout<<"physicsGroup.particles.size="<<physicsGroup.particles.size()<<std::endl;
  const size_t offset_basket = 4;
  const size_t offset_balloon = 8;
  for (size_t i = 0; i < 4; i++) {
    std::cout<<"Creating rope from "<<(offset_basket + i)<<" to "<<(offset_balloon + i)<<std::endl;
    breathe::physics::verlet::Particle& lower = physicsGroup.particles[offset_basket + i];
    breathe::physics::verlet::Particle& upper = physicsGroup.particles[offset_balloon + i];

    CreateRope(lower, upper);
  }

  // Link the basket to the ground with a rope
  {
    // Create a particle that we can pin to the ground
    breathe::physics::verlet::Particle p0(attachmentPointOnGround);
    physicsGroup.particles.push_back(p0);

    breathe::physics::verlet::Particle& groundParticle = physicsGroup.particles.back(); // The point to pin the rope to the ground
    breathe::physics::verlet::Particle& basketParticle = physicsGroup.particles[0];     // A point on the bottom of the basket

    CreateRope(groundParticle, basketParticle);

    // Pin the ground particle to hold it in position
    physicsGroup.pins.push_back(&groundParticle);
  }

  std::cout<<"nParticles="<<nParticles<<", nSprings="<<nSprings<<", nRopes="<<nRopes<<std::endl;
  std::cout<<"physicsGroup.particles.size="<<physicsGroup.particles.size()<<", physicsGroup.springs.size="<<physicsGroup.springs.size()<<std::endl;

  Update(context);

  return true;
}

void cHotAirBalloon::Update(opengl::cContext& context)
{
  physicsWorld.Update();

  // Apply lift force to the balloon
  const size_t balloonOffset = 8;
  for (size_t i = 0; i < 8; i++) {
    physicsGroup.particles[balloonOffset + i].pos.y += fBalloonLift;
  }

  // Update the physics for the grass
  breathe::physics::verlet::Update(physicsWorld, physicsGroup);

  // Collide the particles with the ground
  breathe::physics::verlet::CollideGroundPlane(physicsGroup, 0.0f);


  // Update our basket and balloon positions, we get the average point of the 8 particles for each box
  basketCentrePoint = (
    (physicsGroup.particles[0].pos + physicsGroup.particles[1].pos + physicsGroup.particles[2].pos + physicsGroup.particles[3].pos) +
    (physicsGroup.particles[4].pos + physicsGroup.particles[5].pos + physicsGroup.particles[6].pos + physicsGroup.particles[7].pos)
  ) / 8;
  balloonCentrePoint = (
    (physicsGroup.particles[8].pos + physicsGroup.particles[9].pos + physicsGroup.particles[10].pos + physicsGroup.particles[11].pos) +
    (physicsGroup.particles[12].pos + physicsGroup.particles[13].pos + physicsGroup.particles[14].pos + physicsGroup.particles[15].pos)
  ) / 8;


  // Work out the rotation of the balloon based on the direction of the a corner
  {
    // Get the 3 edges coming out of the first corner, this gives us very approximate directions for the right, up, forward (XYZ) orientations of the whole cube
    // https://stackoverflow.com/questions/28002400/find-angle-of-rotation-of-cube
    // NOTE: This is highly inaccurate as the points can all be squished out of shape even if only temporarily, so we can easily not have nice right angles for a few frames
    const spitfire::math::cVec3 right((physicsGroup.particles[8].pos - physicsGroup.particles[11].pos).GetNormalised());
    const spitfire::math::cVec3 up((physicsGroup.particles[12].pos - physicsGroup.particles[8].pos).GetNormalised());
    const spitfire::math::cVec3 forward((physicsGroup.particles[8].pos - physicsGroup.particles[9].pos).GetNormalised());

    balloonRotation.SetEntries(
      right.x, right.y, right.z, 0.0f,
      up.x, up.y, up.z, 0.0f,
      forward.x, forward.y, forward.z, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    );
  }



  // Discard and recreate our basket VBO
  if (vboBasket.IsCompiled()) context.DestroyStaticVertexBufferObject(vboBasket);

  {
    opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

    opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

    // Lower
    const spitfire::math::cVec3& p0 = physicsGroup.particles[0].pos;
    const spitfire::math::cVec3& p1 = physicsGroup.particles[1].pos;
    const spitfire::math::cVec3& p2 = physicsGroup.particles[2].pos;
    const spitfire::math::cVec3& p3 = physicsGroup.particles[3].pos;

    // Upper
    const spitfire::math::cVec3& p4 = physicsGroup.particles[4].pos;
    const spitfire::math::cVec3& p5 = physicsGroup.particles[5].pos;
    const spitfire::math::cVec3& p6 = physicsGroup.particles[6].pos;
    const spitfire::math::cVec3& p7 = physicsGroup.particles[7].pos;

    const spitfire::math::cVec2 t0(0.0f, 1.0f);
    const spitfire::math::cVec2 t1(1.0f, 1.0f);
    const spitfire::math::cVec2 t2(0.0f, 0.0f);
    const spitfire::math::cVec2 t3(1.0f, 0.0f);

    const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);

    util::AddQuad(builder, p0, p1, p4, p5, t0, t1, t2, t3, normal);
    util::AddQuad(builder, p1, p2, p5, p6, t0, t1, t2, t3, normal);
    util::AddQuad(builder, p2, p3, p6, p7, t0, t1, t2, t3, normal);
    util::AddQuad(builder, p3, p0, p7, p4, t0, t1, t2, t3, normal);

    util::AddQuad(builder, p1, p0, p2, p3, t0, t1, t2, t3, normal);
    util::AddQuad(builder, p4, p5, p7, p6, t0, t1, t2, t3, normal);

    vboBasket.SetData(pGeometryDataPtr);

    vboBasket.Compile();
  }






  if (bDebug) {
    // For debugging where the physics particle geometry making up the balloon box

    // Discard and recreate our balloon VBO
    if (vboBalloonDebug.IsCompiled()) context.DestroyStaticVertexBufferObject(vboBalloonDebug);

    {
      opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

      opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

      // Lower
      const spitfire::math::cVec3& p0 = physicsGroup.particles[8].pos;
      const spitfire::math::cVec3& p1 = physicsGroup.particles[9].pos;
      const spitfire::math::cVec3& p2 = physicsGroup.particles[10].pos;
      const spitfire::math::cVec3& p3 = physicsGroup.particles[11].pos;

      // Upper
      const spitfire::math::cVec3& p4 = physicsGroup.particles[12].pos;
      const spitfire::math::cVec3& p5 = physicsGroup.particles[13].pos;
      const spitfire::math::cVec3& p6 = physicsGroup.particles[14].pos;
      const spitfire::math::cVec3& p7 = physicsGroup.particles[15].pos;

      const spitfire::math::cVec2 t0(0.0f, 1.0f);
      const spitfire::math::cVec2 t1(1.0f, 1.0f);
      const spitfire::math::cVec2 t2(0.0f, 0.0f);
      const spitfire::math::cVec2 t3(1.0f, 0.0f);

      const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);

      util::AddQuad(builder, p0, p1, p4, p5, t0, t1, t2, t3, normal);
      util::AddQuad(builder, p1, p2, p5, p6, t0, t1, t2, t3, normal);
      util::AddQuad(builder, p2, p3, p6, p7, t0, t1, t2, t3, normal);
      util::AddQuad(builder, p3, p0, p7, p4, t0, t1, t2, t3, normal);

      util::AddQuad(builder, p1, p0, p2, p3, t0, t1, t2, t3, normal);
      util::AddQuad(builder, p4, p5, p7, p6, t0, t1, t2, t3, normal);

      vboBalloonDebug.SetData(pGeometryDataPtr);

      vboBalloonDebug.Compile();
    }
  }



  // Discard and recreate our dynamic rope VBO
  if (vboRope.IsCompiled()) context.DestroyStaticVertexBufferObject(vboRope);


  // Create the rope geometry
  // There are:
  // 4 ropes connecting the balloon to the basket, and
  // 1 rope attaching the basket to the ground
  //
  // We could iterate over the rope particles, but the particle we need aren't sequential in the particles vector, so instead,
  // we iterate over the rope springs instead which are sequential and at the end of the springs vector

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

#if 1
  const float fRopeWidth = 0.3f;
  const float fHalfRopeWidth = 0.5f * fRopeWidth;

  // For each rope segment
  for (auto&& rope : physicsGroup.ropes) {
    // TODO: Add a cube
    // TODO: Rotate it
    // TODO: Change it to an 8 sided cylinder
    const spitfire::math::cVec3& particle0 = rope.a->pos;
    const spitfire::math::cVec3& particle1 = rope.b->pos;

    const spitfire::math::cVec3 p0(particle0 + spitfire::math::cVec3(-fHalfRopeWidth, 0.0f, 0.0f));
    const spitfire::math::cVec3 p1(particle0 + spitfire::math::cVec3(fHalfRopeWidth, 0.0f, 0.0f));
    const spitfire::math::cVec3 p2(particle1 + spitfire::math::cVec3(-fHalfRopeWidth, 0.0f, 0.0f));
    const spitfire::math::cVec3 p3(particle1 + spitfire::math::cVec3(fHalfRopeWidth, 0.0f, 0.0f));

    const spitfire::math::cVec2 t0(0.0f, 1.0f);
    const spitfire::math::cVec2 t1(1.0f, 1.0f);
    const spitfire::math::cVec2 t2(0.0f, 0.0f);
    const spitfire::math::cVec2 t3(1.0f, 0.0f);

    const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);

    // Add a front facing quad
    builder.PushBack(p1, normal, t1);
    builder.PushBack(p2, normal, t2);
    builder.PushBack(p3, normal, t3);
    builder.PushBack(p2, normal, t2);
    builder.PushBack(p1, normal, t1);
    builder.PushBack(p0, normal, t0);
  }
#else
  const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);

  // Debug wireframe of the verlet geometry
  for (auto&& spring : physicsGroup.springs) {
    const spitfire::math::cVec3 p0 = spring.a->pos;
    const spitfire::math::cVec3 p1 = spring.b->pos;

    // TODO: Work out the texturing
    const spitfire::math::cVec2 t0(0.0f, 0.0f);
    const spitfire::math::cVec2 t1(1.0f, 1.0f);

    builder.PushBack(p0, normal, t0); builder.PushBack(p1, normal, t1);
  }

  for (auto&& rope : physicsGroup.ropes) {
    const spitfire::math::cVec3 p0 = rope.a->pos;
    const spitfire::math::cVec3 p1 = rope.b->pos;

    // TODO: Work out the texturing
    const spitfire::math::cVec2 t0(0.0f, 0.0f);
    const spitfire::math::cVec2 t1(1.0f, 1.0f);

    builder.PushBack(p0, normal, t0); builder.PushBack(p1, normal, t1);
  }
#endif

  context.CreateStaticVertexBufferObject(vboRope);

  vboRope.SetData(pGeometryDataPtr);
  vboRope.Compile();
}

void cHotAirBalloon::Destroy(opengl::cContext& context)
{
  context.DestroyStaticVertexBufferObject(vboBalloonDebug);
  context.DestroyStaticVertexBufferObject(vboBalloon);
  context.DestroyStaticVertexBufferObject(vboBasket);
  context.DestroyStaticVertexBufferObject(vboRope);
  context.DestroyTexture(textureBalloon);
  context.DestroyTexture(textureBasket);
  context.DestroyTexture(textureRope);
}

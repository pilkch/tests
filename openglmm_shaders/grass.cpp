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
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Application headers
#include "grass.h"

// https://www.reddit.com/r/Unity3D/comments/pks6ys/i_learned_about_verlet_integration_thanks_to/hilra7c/?context=8&depth=9

// We create "radio towers" of verlet particles joined by springs. The sides are made of squares, the cross section is a triangle. I ended up having to reinforce them with diagonal supports because it keep collapsing when brushed even with stiff springs.
//
// From the front (The renderable geometry is created on one side of the "radio tower", the front faces only, the only difference between the verlet version and the renderable version is that
//   the top points taper into a single point in the renderable geometry so that get a nice grass blade tip):
//
//   +---+
//   | X |
//   +---+
//   | X |
//   +---+
//   | X |
//   +---+
//
// From above (The front faces are at the bottom of this diagram):
//
//     +
//    | |
//   |   |
//  +--+--+
//

namespace {

// Our grid of grass blades
const size_t gridRows = 10;
const size_t gridColumns = 10;

const size_t points_vertical = 4;
const size_t springs_vertical = 3;

}

spitfire::math::cAABB3 cGrass::GetBoundingBox() const
{
  const spitfire::math::cVec2 gridScale(0.5f, 0.5f);
  const float fHalfWidth = 0.5f * gridScale.x * float(gridRows);
  const float fHalfDepth = 0.5f * gridScale.y * float(gridColumns);
  const float fHeight = 1.0f;
  const spitfire::math::cVec3 center(20.0f, 0.0f, 10.0f);
  spitfire::math::cAABB3 boundingBox;
  boundingBox.SetMinMax(center - spitfire::math::cVec3(fHalfWidth, 0.0f, fHalfDepth), center + spitfire::math::cVec3(fHalfWidth, fHeight, fHalfDepth));
  return boundingBox;
}

bool cGrass::Init(opengl::cContext& context, breathe::physics::verlet::cWorld& physicsWorld)
{
  context.CreateShader(shader, TEXT("shaders/grass.vert"), TEXT("shaders/grass.frag"));

  context.CreateTexture(texture, TEXT("textures/grass.png"));
  if (texture.GetPixelFormat() != voodoo::PIXELFORMAT::R8G8B8A8) {
    return false;
  }

  // Create the particles and springs for each grass tuft
  // Each grass tuft is made up of 4 particles in a row with 3 springs between them
  const float segmentsHeight[3] = { 0.5f, 0.25f, 0.25f };
  const float jointStiffness[3] = { 0.98f, 0.85f, 0.8f };


  // Create the grass geometry
  const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);
  const spitfire::math::cVec3 axisY(0.0f, 1.0f, 0.0f);

  spitfire::math::cScopedPredictableRandom randomGenerator(4754);

  // We create our geometry by creating a grid of grass at a set scale, then jittering the position of every bit of grass slightly -0.5..0.5
  // It's not the best, but is fast and simple

  // The scale of the grid
  const spitfire::math::cVec2 gridScale(0.5f, 0.5f);

  // The middle of the grass field
  const spitfire::math::cVec3 corner(spitfire::math::cVec3(20.0f, 0.0f, 10.0f) + (-0.5f * spitfire::math::cVec3(gridScale.x * float(gridRows), 0.0f, gridScale.y * float(gridColumns))));

  for (size_t y = 0; y < gridRows; y++) {
    for (size_t x = 0; x < gridColumns; x++) {
      // Randomise the position
      const spitfire::math::cVec2 randomOffset2D(-0.5f + randomGenerator.GetRandomNumber0To1(), -0.5f +  + randomGenerator.GetRandomNumber0To1());
      const spitfire::math::cVec2 position2D(gridScale * (spitfire::math::cVec2(x, y) + randomOffset2D));
      const spitfire::math::cVec3 position = corner + spitfire::math::cVec3(position2D.x, 0.0f, position2D.y);

      // Randomise the size
      const float fWidth = 0.3f + (0.1f * randomGenerator.GetRandomNumber0To1());
      const float fHeightScale = 1.0f + (0.3f * randomGenerator.GetRandomNumber0To1());
      const float fHalfWidth = 0.5f * fWidth;
      const float fTriangularDepth = fWidth * 0.86603f; // For an equilateral triangle the height down the middle from an edge to the opposite point is 0.86603

      // Randomise the rotation
      const float fRotationDegreesOfGrass = randomGenerator.GetRandomNumber0To1() * 360.0f;

      // The rotation for this billboard of grass
      spitfire::math::cQuaternion rotationY;
      rotationY.SetFromAxisAngleDegrees(axisY, fRotationDegreesOfGrass);

      spitfire::math::cMat4 matRotation;
      matRotation.SetRotation(rotationY);

      const size_t points_offset = physicsGroup.particles.size();
      std::cout<<"points_offset: "<<points_offset<<std::endl;

      // Create the points for this grass blade
      float fY = 0.0f;

      // Create a triangle for the ground layer
      {
        const spitfire::math::cVec3 trianglePoints[3] = {
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(-fHalfWidth, fY, 0.0f)),      // Front left
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, fY, fTriangularDepth)), // At the back at the center
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(fHalfWidth, fY, 0.0f)),       // Front right
        };

        breathe::physics::verlet::Particle p0(trianglePoints[0]);
        physicsGroup.particles.push_back(p0);
        breathe::physics::verlet::Particle p1(trianglePoints[1]);
        physicsGroup.particles.push_back(p1);
        breathe::physics::verlet::Particle p2(trianglePoints[2]);
        physicsGroup.particles.push_back(p2);
      }

      // Now add a triangle for all the other layers
      for (size_t i = 1; i < points_vertical; i++) {
        fY += fHeightScale * segmentsHeight[i - 1];

        // Create a triangle for this layer
        const spitfire::math::cVec3 trianglePoints[3] = {
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(-fHalfWidth, fY, 0.0f)),      // Front left
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, fY, fTriangularDepth)), // At the back at the center
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(fHalfWidth, fY, 0.0f)),       // Front right
        };

        breathe::physics::verlet::Particle p0(trianglePoints[0]);
        physicsGroup.particles.push_back(p0);
        breathe::physics::verlet::Particle p1(trianglePoints[1]);
        physicsGroup.particles.push_back(p1);
        breathe::physics::verlet::Particle p2(trianglePoints[2]);
        physicsGroup.particles.push_back(p2);
      }
    }
  }

  for (size_t i = 0; i < 12; i++) {
    const spitfire::math::cVec3 p = physicsGroup.particles[i].pos;
    std::cout<<"particle["<<i<<"]: "<<p.x<<", "<<p.y<<", "<<p.z<<std::endl;
  }

  for (size_t y = 0; y < gridRows; y++) {
    for (size_t x = 0; x < gridColumns; x++) {
      const size_t points_offset = ((y * gridColumns) + x) * (3 * points_vertical);
      std::cout<<"points_offset: "<<points_offset<<std::endl;

      // Pin the lower particles to the ground
      for (size_t i = 0; i < 3; i++) {
        physicsGroup.pins.push_back(&physicsGroup.particles[points_offset + i]);
      }

      // Link each layer of particles to each other particle in the triangle with springs and link all the layers
      for (size_t i = 0; i < points_vertical - 1; i++) {
        const float fSegmentStiffness = jointStiffness[i];

        const size_t points_layer_offset = points_offset + (i * 3);
        std::cout<<"Adding springs for this layer and next layer "<<points_layer_offset<<std::endl;
        // The current layer
        breathe::physics::verlet::Particle* a = &physicsGroup.particles[points_layer_offset];
        breathe::physics::verlet::Particle* b = &physicsGroup.particles[points_layer_offset + 1];
        breathe::physics::verlet::Particle* c = &physicsGroup.particles[points_layer_offset + 2];

        // The next layer above us
        breathe::physics::verlet::Particle* d = &physicsGroup.particles[points_layer_offset + 3];
        breathe::physics::verlet::Particle* e = &physicsGroup.particles[points_layer_offset + 4];
        breathe::physics::verlet::Particle* f = &physicsGroup.particles[points_layer_offset + 5];

        // Link each layer of particles to each other particle in the triangle with springs
        const float ab = (a->pos - b->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, b, ab, fSegmentStiffness));
        const float bc = (b->pos - c->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(b, c, bc, fSegmentStiffness));
        const float ca = (c->pos - a->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(c, a, ca, fSegmentStiffness));

        // Link this layer to the particles in the layer above
        const float ad = (a->pos - d->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, d, ad, fSegmentStiffness));
        const float be = (b->pos - e->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(b, e, be, fSegmentStiffness));
        const float cf = (c->pos - f->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(c, f, cf, fSegmentStiffness));

        // The "radio towers" were falling over and collapsing, so we add a spring from each point on this triangle, diagonally up to each neighouring corner on the next layer
        const float ae = (a->pos - e->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, e, ae, fSegmentStiffness));
        const float bf = (b->pos - f->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(b, f, bf, fSegmentStiffness));
        const float cd = (c->pos - d->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(c, d, cd, fSegmentStiffness));
        const float af = (a->pos - f->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, f, af, fSegmentStiffness));
        const float bd = (b->pos - d->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(b, d, bd, fSegmentStiffness));
        const float ce = (c->pos - e->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(c, e, ce, fSegmentStiffness));
      }

      // Link the last row
      for (size_t i = points_vertical - 1; i < points_vertical; i++) {
        const float fSegmentStiffness = jointStiffness[2];

        const size_t points_layer_offset = points_offset + (i * 3);
        std::cout<<"Adding springs for layer "<<points_layer_offset<<std::endl;
        breathe::physics::verlet::Particle* a = &physicsGroup.particles[points_layer_offset];
        breathe::physics::verlet::Particle* b = &physicsGroup.particles[points_layer_offset + 1];
        breathe::physics::verlet::Particle* c = &physicsGroup.particles[points_layer_offset + 2];

        // Link each layer of particles to each other particle in the triangle with springs
        const float ab = (a->pos - b->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(a, b, ab, fSegmentStiffness));
        const float bc = (b->pos - c->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(b, c, bc, fSegmentStiffness));
        const float ca = (c->pos - a->pos).GetLength();
        physicsGroup.springs.push_back(breathe::physics::verlet::Spring(c, a, ca, fSegmentStiffness));
      }
    }
  }

  const spitfire::math::cVec3 dummyCameraPosition(0.0f, 1000.0f, 0.0f);
  Update(context, dummyCameraPosition, physicsWorld);

  return true;
}

void cGrass::Update(opengl::cContext& context, const spitfire::math::cVec3& cameraPosition, breathe::physics::verlet::cWorld& physicsWorld)
{
  // Update the physics for the grass
  breathe::physics::verlet::Update(physicsWorld, physicsGroup);

  // Center the player capsule with the tip at the camera position
  const float fPlayerRadius = 0.5f;
  const float fPlayerHeight = 1.0f;

  spitfire::math::cCapsule playerCapsule;
  playerCapsule.SetBase(cameraPosition - spitfire::math::cVec3(0.0f, fPlayerHeight, 0.0f));
  playerCapsule.SetTip(cameraPosition);
  playerCapsule.SetRadius(fPlayerRadius);

  // Collide the particles with the player
  breathe::physics::verlet::Collide(physicsGroup, playerCapsule);

  // Collide the particles with the ground
  breathe::physics::verlet::CollideGroundPlane(physicsGroup, 0.0f);


  // Discard and recreate our VBO
  if (vbo.IsCompiled()) context.DestroyStaticVertexBufferObject(vbo);
  context.CreateStaticVertexBufferObject(vbo);

  // Create the grass geometry

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v3_n3_t2_c4 builder(*pGeometryDataPtr);

  const spitfire::math::cVec3 axisY(0.0f, 1.0f, 0.0f);

  spitfire::math::cScopedPredictableRandom randomGenerator(4754);

#if 1
  const size_t nGrassBlades = gridRows * gridColumns;

  for (size_t i = 0; i < nGrassBlades; i++) {
    const size_t points_offset = i * (3 * points_vertical);

    spitfire::math::cColourHSL colourHSL;
    colourHSL.fHue0To360 = randomGenerator.randomf(87.0f / 360.0f, 135.0f / 360.0f);
    colourHSL.fSaturation0To1 = randomGenerator.randomf(0.8f, 0.9f);
    colourHSL.fLuminance0To1 = randomGenerator.randomf(0.24f, 0.38f);

    const spitfire::math::cColour colour(colourHSL.GetRGBA());

    const spitfire::math::cVec3& p0 = physicsGroup.particles[points_offset + 0].pos;
    const spitfire::math::cVec3& p1 = physicsGroup.particles[points_offset + 2].pos;
    const spitfire::math::cVec3& p2 = physicsGroup.particles[points_offset + 3].pos;
    const spitfire::math::cVec3& p3 = physicsGroup.particles[points_offset + 5].pos;
    const spitfire::math::cVec3& p4 = physicsGroup.particles[points_offset + 6].pos;
    const spitfire::math::cVec3& p5 = physicsGroup.particles[points_offset + 8].pos;
    const spitfire::math::cVec3& p6 = physicsGroup.particles[points_offset + 9].pos;
    const spitfire::math::cVec3& p7 = physicsGroup.particles[points_offset + 11].pos;

    // NOTE: We only create one side of the geometry but we render with culling disabled so that we render both sides

    const float fSegmentTextureHeight = 1.0f / float(springs_vertical);

    const spitfire::math::cVec2 t0(0.0f, 3 * fSegmentTextureHeight);
    const spitfire::math::cVec2 t1(1.0f, 3 * fSegmentTextureHeight);
    const spitfire::math::cVec2 t2(0.0f, 2 * fSegmentTextureHeight);
    const spitfire::math::cVec2 t3(1.0f, 2 * fSegmentTextureHeight);
    const spitfire::math::cVec2 t4(0.0f, fSegmentTextureHeight);
    const spitfire::math::cVec2 t5(1.0f, fSegmentTextureHeight);
    const spitfire::math::cVec2 t6(0.0f, 0.0f);
    const spitfire::math::cVec2 t7(1.0f, 0.0f);

    // TODO: I guess we can work out the normal from 3 surrounding vertices?
    const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);

    // Add a front facing quad
    builder.PushBack(p1, normal, t1, colour);
    builder.PushBack(p2, normal, t2, colour);
    builder.PushBack(p3, normal, t3, colour);
    builder.PushBack(p2, normal, t2, colour);
    builder.PushBack(p1, normal, t1, colour);
    builder.PushBack(p0, normal, t0, colour);

    // Add a front facing quad
    builder.PushBack(p3, normal, t3, colour);
    builder.PushBack(p4, normal, t4, colour);
    builder.PushBack(p5, normal, t5, colour);
    builder.PushBack(p3, normal, t3, colour);
    builder.PushBack(p2, normal, t2, colour);
    builder.PushBack(p4, normal, t4, colour);

    // Add a front facing quad
    builder.PushBack(p5, normal, t5, colour);
    builder.PushBack(p6, normal, t6, colour);
    builder.PushBack(p7, normal, t7, colour);
    builder.PushBack(p4, normal, t4, colour);
    builder.PushBack(p6, normal, t6, colour);
    builder.PushBack(p5, normal, t5, colour);
  }
#else
  const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);
  const spitfire::math::cColour colour(1.0f, 0.0f, 0.0f);

  // Debug wireframe of the verlet geometry
  for (auto&& spring : physicsGroup.springs) {
    const spitfire::math::cVec3 p0 = spring.a->pos;
    const spitfire::math::cVec3 p1 = spring.b->pos;

    builder.PushBack(p0, normal, colour); builder.PushBack(p1, normal, colour);
  }
#endif


  context.CreateStaticVertexBufferObject(vbo);

  vbo.SetData(pGeometryDataPtr);
  vbo.Compile();
}

void cGrass::Destroy(opengl::cContext& context)
{
  context.DestroyStaticVertexBufferObject(vbo);
  context.DestroyTexture(texture);
  context.DestroyShader(shader);
}

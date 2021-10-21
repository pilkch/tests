// Standard headers
#include <cassert>
#include <iostream>

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/math/math.h>

// Application headers
#include "verlet_physics.h"

namespace verlet_physics {

// ** cWorld

cWorld::cWorld() :
  updatesSinceLastMajorChange(0),
  updatesSinceLastMinorChange(0),
  gravity(-9.8f)
{
  spitfire::math::cRand rng;

  // General wind direction
  const spitfire::math::cVec3 generalWind(0.3f, 0.0f, 0.3f);

  const float fMaxHorizontalForce = 0.1f;
  const float fMaxVerticalForce = 0.1f;

  // Add some randomish major events
  for (size_t i = 0; i < 10; i++) {
    majorWindEvents.push_back(generalWind + spitfire::math::cVec3(fMaxHorizontalForce * rng.randomMinusOneToPlusOnef(), fMaxVerticalForce * rng.randomMinusOneToPlusOnef(), fMaxHorizontalForce * rng.randomMinusOneToPlusOnef()));
  }

  // Add some minor fluctuations
  for (size_t i = 0; i < 20; i++) {
    minorWindFluctuations.push_back(0.1f * spitfire::math::cVec3(fMaxHorizontalForce * rng.randomMinusOneToPlusOnef(), fMaxVerticalForce * rng.randomMinusOneToPlusOnef(), fMaxHorizontalForce * rng.randomMinusOneToPlusOnef()));
  }
}

void cWorld::Update()
{
  // This is a very basic model, it's basically minimum viable wind generation for a basic demo
  // We cycle through the major wind events, changing to the next one every 3 seconds
  // We cycle through the minor wind fluctuations, changing to the next one every 0.5 seconds
  // The current weather is calculated as just major event + interpolation to the next major event + minor fluctuation

  const size_t updatesPerSecond = 60;
  const size_t majorChangeAtUpdates = 3 * updatesPerSecond; // Change every 3 seconds
  const size_t minorChangeAtUpdates = (updatesPerSecond>>1); // Change every 0.5 seconds

  updatesSinceLastMajorChange++;
  updatesSinceLastMinorChange++;

  if (updatesSinceLastMajorChange > majorChangeAtUpdates) {
    // Cycle to the next major event
    const spitfire::math::cVec3 current = *majorWindEvents.begin();
    majorWindEvents.pop_front();
    majorWindEvents.push_back(current);

    updatesSinceLastMajorChange = 0;
  }

  if (updatesSinceLastMinorChange > minorChangeAtUpdates) {
    // Cycle to the next minor event
    const spitfire::math::cVec3 current = *minorWindFluctuations.begin();
    minorWindFluctuations.pop_front();
    minorWindFluctuations.push_back(current);

    updatesSinceLastMinorChange = 0;
  }
}


// ** cWavingFlag

void cWavingFlag::Init(float fWidthMeters, float fHeightMeters, size_t points_horizontal, size_t points_vertical)
{
  const float fSegmentWidth = fWidthMeters / float(points_horizontal);
  const float fSegmentHeight = fHeightMeters / float(points_vertical);

  // Add points from the top left to the bottom right
  for (size_t y = 0; y < points_vertical; y++) {
    for (size_t x = 0; x < points_horizontal; x++) {
      Particle p(spitfire::math::cVec3(float(x) * fSegmentWidth, fHeightMeters - (float(y) * fSegmentHeight), 0.0f));
      particles.push_back(p);
    }
  }

  // Pin the left hand particles to their current positions
  for (size_t y = 0; y < points_vertical; y++) {
    pins.push_back(&particles[(y * points_horizontal)]);
  }


  const float fStiffness = 0.8f;

  // Link each particle together with springs
  for (size_t y = 0; y < points_vertical - 1; y++) {
    for (size_t x = 0; x < points_horizontal - 1; x++) {
      // Create a spring from this particle to the particle on the right
      {
        Particle* a = &particles[(y * points_horizontal) + x];
        Particle* b = &particles[(y * points_horizontal) + x + 1];
        const float fDistance = (a->pos - b->pos).GetLength();
        springs.push_back(Spring(a, b, fDistance, fStiffness));
      }

      // Create a spring from this particle to the particle below us
      {
        Particle* a = &particles[(y * points_horizontal) + x];
        Particle* b = &particles[((y + 1) * points_horizontal) + x];
        const float fDistance = (a->pos - b->pos).GetLength();
        springs.push_back(Spring(a, b, fDistance, fStiffness));
      }
    }
  }

  // Link the last row
  for (size_t x = 0; x < points_horizontal - 1; x++) {
    // Create a spring from this particle to the particle on the right
    const size_t y = points_vertical - 1;
    Particle* a = &particles[(y * points_horizontal) + x];
    Particle* b = &particles[(y * points_horizontal) + x + 1];
    const float fDistance = (a->pos - b->pos).GetLength();
    springs.push_back(Spring(a, b, fDistance, fStiffness));
  }

  // Link the last column
  for (size_t y = 0; y < points_vertical - 1; y++) {
    // Create a spring from this particle to the particle below us
    const size_t x = points_horizontal - 1;
    Particle* a = &particles[(y * points_horizontal) + x];
    Particle* b = &particles[((y + 1) * points_horizontal) + x];
    const float fDistance = (a->pos - b->pos).GetLength();
    springs.push_back(Spring(a, b, fDistance, fStiffness));
  }
}

void cWavingFlag::Update(const cWorld& world)
{
  const spitfire::math::cVec3 forces = 0.0001f * world.GetGravity() + 0.01f * world.GetWind();
  //const spitfire::math::cVec3 forces = spitfire::math::cVec3();

	const float fFriction = 0.99f;

  // Apply forces
  for (auto& p : particles) {
    // Calculate velocity
    const spitfire::math::cVec3 velocity = fFriction * (p.pos - p.lastPos);

    // Save last good state
    p.lastPos = p.pos;

    // Add forces
    p.pos += forces;

    // Apply inertia
    p.pos += velocity;
  }

  // Apply constraints
  const float relaxationSteps = 16;
	const float fStepCoefficient = 1.0f / float(relaxationSteps);

	// Apply spring constraints with relaxing
  for (size_t i = 0; i < relaxationSteps; ++i) {
    for (auto& spring : springs) {
      const spitfire::math::cVec3 normal = spring.a->pos - spring.b->pos;
      const float m = normal.GetLength() * normal.GetLength();
      const float fScale = ((spring.fDistance * spring.fDistance - m) / m) * spring.fStiffness * fStepCoefficient;
      const spitfire::math::cVec3 offset = fScale * normal;
      spring.a->pos += offset;
      spring.b->pos -= offset;
    }
	}

  // Apply our pin constraints (Move all pinned points back to their starting positions)
  for (auto& pin : pins) {
    pin->pos = pin->lastPos;
  }
}

}

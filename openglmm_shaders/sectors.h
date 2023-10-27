#pragma once

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/geometry.h>
#include <spitfire/util/timer.h>

class cSectors
{
public:
  void AddSectorBoundingBox(const std::string& name, const spitfire::math::cAABB3& boundingBox);
  void AddSectorSphere(const std::string& name, const spitfire::math::cVec3 position, float fRadius);
  void UpdateSectorSphere(const std::string& name, const spitfire::math::cVec3 position, float fRadius);

  std::string GetClosestSectorRaycast(const spitfire::math::cRay3& ray) const;

private:
  struct SectorBoundingBox {
    std::string name;
    spitfire::math::cAABB3 boundingBox;
  };

  struct SectorSphere {
    std::string name;
    spitfire::math::cSphere boundingSphere;
  };

  std::vector<SectorBoundingBox> boundingBoxSectors;
  std::vector<SectorSphere> sphereSectors;
};

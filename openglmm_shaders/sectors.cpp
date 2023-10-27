#include <cassert>
#include <cmath>

#include <vector>
#include <string>

// Application headers
#include "sectors.h"

void cSectors::AddSectorBoundingBox(const std::string& name, const spitfire::math::cAABB3& boundingBox)
{
  SectorBoundingBox sector;
  sector.name = name;
  sector.boundingBox = boundingBox;

  boundingBoxSectors.push_back(sector);
}

void cSectors::AddSectorSphere(const std::string& name, const spitfire::math::cVec3 position, float fRadius)
{
  SectorSphere sector;
  sector.name = name;
  sector.boundingSphere.SetPosition(position);
  sector.boundingSphere.SetRadius(fRadius);

  sphereSectors.push_back(sector);
}

void cSectors::UpdateSectorSphere(const std::string& name, const spitfire::math::cVec3 position, float fRadius)
{
  for (auto&& sector : sphereSectors) {
    if (sector.name == name) {
      sector.boundingSphere.SetPosition(position);
      sector.boundingSphere.SetRadius(fRadius);
      return;
    }
  }

  // We didn't find it, so add a new one
  AddSectorSphere(name, position, fRadius);
}

std::string cSectors::GetClosestSectorRaycast(const spitfire::math::cRay3& ray) const
{
  struct Closest {
    std::string name;
    float fDistance;
  };

  Closest closest;
  closest.fDistance = spitfire::math::cINFINITY;

  float fDepth = 0.0f;

  for (auto&& sector : boundingBoxSectors) {
    if (ray.CollideWithAABB(sector.boundingBox, fDepth)) {
      if (fDepth < closest.fDistance) {
        closest.name = sector.name;
        closest.fDistance = fDepth;
      }
    }
  }

  for (auto&& sector : sphereSectors) {
    if (ray.CollideWithSphere(sector.boundingSphere, fDepth)) {
      if (fDepth < closest.fDistance) {
        closest.name = sector.name;
        closest.fDistance = fDepth;
      }
    }
  }

  return closest.name;
}

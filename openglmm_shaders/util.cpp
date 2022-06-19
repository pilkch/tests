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
#include <spitfire/util/timer.h>
#include <spitfire/util/thread.h>

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

// Breathe headers
#include <breathe/render/model/cFileFormatOBJ.h>
#include <breathe/render/model/cStatic.h>

// libopenglmm headers
#include <libopenglmm/libopenglmm.h>
#include <libopenglmm/cContext.h>
#include <libopenglmm/cFont.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Application headers
#include "util.h"

// ** cFreeLookCamera

namespace {

const float DEFAULT_YAW = 90.0f;
const float DEFAULT_PITCH = 0.0f;
const float DEFAULT_ZOOM = 45.0f;

}

cFreeLookCamera::cFreeLookCamera() :
  fRotationRight(DEFAULT_YAW),
  fRotationUp(DEFAULT_PITCH),
  fZoom(DEFAULT_ZOOM)
{
}

spitfire::math::cVec3 cFreeLookCamera::GetPosition() const
{
  return position;
}

void cFreeLookCamera::SetPosition(const spitfire::math::cVec3& _position)
{
  position = _position;
}

void cFreeLookCamera::SetRotation(const spitfire::math::cQuaternion& rotation)
{
  const spitfire::math::cVec3 euler = rotation.GetEuler();
  fRotationRight = euler.x;
  fRotationUp = euler.z;
}

void cFreeLookCamera::LookAt(const spitfire::math::cVec3& eye, const spitfire::math::cVec3& target, const spitfire::math::cVec3 up)
{
  const spitfire::math::cMat4 matView = spitfire::math::cMat4::LookAt(eye, target, up);

  SetPosition(eye);
  SetRotation(matView.GetRotation());
}

void cFreeLookCamera::MoveX(float xmmod)
{
  const spitfire::math::cQuaternion rotation = GetRotation();
  position += (-rotation) * spitfire::math::cVec3(xmmod, 0.0f, 0.0f);
}

void cFreeLookCamera::MoveZ(float ymmod)
{
  const spitfire::math::cQuaternion rotation = GetRotation();
  position += (-rotation) * spitfire::math::cVec3(0.0f, 0.0f, -ymmod);
}

void cFreeLookCamera::RotateX(float xrmod)
{
  fRotationRight += xrmod;
}

void cFreeLookCamera::RotateY(float yrmod)
{
  fRotationUp += yrmod;
}

spitfire::math::cQuaternion cFreeLookCamera::GetRotation() const
{
  spitfire::math::cQuaternion up;
  up.SetFromAxisAngle(spitfire::math::cVec3(1.0f, 0.0f, 0.0f), spitfire::math::DegreesToRadians(fRotationUp));
  spitfire::math::cQuaternion right;
  right.SetFromAxisAngle(spitfire::math::cVec3(0.0f, 1.0f, 0.0f), spitfire::math::DegreesToRadians(fRotationRight));

  return (up * right);
}

spitfire::math::cMat4 cFreeLookCamera::CalculateProjectionMatrix(size_t width, size_t height) const
{
  // TODO: Calculate the FOV from a linear camera style zoom
  const float fFOVDegrees = fZoom;
  return spitfire::math::cMat4::Perspective(spitfire::math::DegreesToRadians(fFOVDegrees), float(width) / float(height), 0.1f, 1000.0f);
}

spitfire::math::cMat4 cFreeLookCamera::CalculateViewMatrix() const
{
  spitfire::math::cMat4 matTranslation;
  matTranslation.TranslateMatrix(-position);

  const spitfire::math::cQuaternion rotation = -GetRotation();
  const spitfire::math::cMat4 matRotation = rotation.GetMatrix();

  spitfire::math::cMat4 matTargetTranslation;
  matTargetTranslation.TranslateMatrix(spitfire::math::cVec3(0.0f, 0.0f, 1.0f));

  return ((matTargetTranslation * matRotation) * matTranslation);
}


spitfire::math::cRay3 CreatePickingRayFromScreenPoint(int screenSpaceX, int screenSpaceY, int screenWidth, int screenHeight, const cFreeLookCamera& camera)
{
  // http://antongerdelan.net/opengl/raycasting.html

  // Normalise device coordinates
  float normalizedDeviceCoordinateX = (2.0f * float(screenSpaceX)) / float(screenWidth) - 1.0f;
  float normalizedDeviceCoordinateY = (2.0f * float(screenSpaceY)) / float(screenHeight) - 1.0f;
  // Homogeneous clip coordinates
  spitfire::math::cVec4 clipSpaceRay = spitfire::math::cVec4(normalizedDeviceCoordinateX, normalizedDeviceCoordinateY, -1.0, 1.0);
  // Eye coordinates
  spitfire::math::cMat4 perspectiveProjectionMatrix = camera.CalculateProjectionMatrix(screenWidth, screenHeight);
  spitfire::math::cVec4 cameraSpaceRay = perspectiveProjectionMatrix.GetInverse() * clipSpaceRay;
  cameraSpaceRay = spitfire::math::cVec4(cameraSpaceRay.x, cameraSpaceRay.y, -1.0, 0.0);

  // World coordinates
  spitfire::math::cMat4 cameraTransformMatrix = camera.CalculateViewMatrix();
  spitfire::math::cVec4 worldSpaceRay = cameraTransformMatrix.GetInverse() * cameraSpaceRay;

  // Normalise
  spitfire::math::cVec3 direction = spitfire::math::cVec3(worldSpaceRay.x, worldSpaceRay.y, worldSpaceRay.z).GetNormalised();

  spitfire::math::cRay3 ray;
  ray.SetOriginAndDirection(camera.GetPosition(), direction);

  return ray;
}


// ** cSimplePostRenderShader

cSimplePostRenderShader::cSimplePostRenderShader(const spitfire::string_t _sName, const spitfire::string_t& _sFragmentShaderFilePath) :
sName(_sName),
sFragmentShaderFilePath(_sFragmentShaderFilePath),
bOn(false)
{
}


namespace util
{
  spitfire::math::cColour ChangeLuminance(const spitfire::math::cColour& colourRGB, float fLuminanceDifferenceMinusOneToPlusOne)
  {
    spitfire::math::cColourHSL colourHSL;
    colourHSL.SetFromRGBA(colourRGB);
    colourHSL.fLuminance0To1 += fLuminanceDifferenceMinusOneToPlusOne;
    return colourHSL.GetRGBA();
  }
}

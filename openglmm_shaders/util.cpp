#include <cassert>
#include <cmath>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>
#include <list>

// OpenGL headers
#include <GL/GLee.h>
#include <GL/glu.h>

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

cFreeLookCamera::cFreeLookCamera() :
fRotationRight(0.0f),
fRotationUp(0.0f)
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
  spitfire::math::cMat4 matView;
  matView.LookAt(eye, target, up);

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

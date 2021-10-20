#ifndef UTIL_H
#define UTIL_H

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

class cFreeLookCamera
{
public:
  cFreeLookCamera();

  spitfire::math::cVec3 GetPosition() const;
  void SetPosition(const spitfire::math::cVec3& position);
  void SetRotation(const spitfire::math::cQuaternion& rotation);

  void LookAt(const spitfire::math::cVec3& eye, const spitfire::math::cVec3& target, const spitfire::math::cVec3 up); // Places the camera at the eye position and looks at the target

  void MoveX(float fDistance);
  void MoveZ(float fDistance);
  void RotateX(float fDegrees);
  void RotateY(float fDegrees);

  spitfire::math::cMat4 CalculateViewMatrix() const;

private:
  spitfire::math::cQuaternion GetRotation() const;

  spitfire::math::cVec3 position;
  float fRotationRight;
  float fRotationUp;
};


class cTextureVBOPair
{
public:
  opengl::cTexture texture;
  opengl::cStaticVertexBufferObject vbo;
};

class cTextureFrameBufferObjectVBOPair
{
public:
  opengl::cTextureFrameBufferObject fbo;
  opengl::cStaticVertexBufferObject vbo;
};

class cShaderVBOPair
{
public:
  opengl::cShader shader;
  opengl::cStaticVertexBufferObject vbo;
};

class cVBOShaderTexturePair
{
public:
  opengl::cStaticVertexBufferObject vbo;
  opengl::cShader shader;
  opengl::cTexture texture;
};

class cSimplePostRenderShader
{
public:
  cSimplePostRenderShader(const spitfire::string_t sName, const spitfire::string_t& sFragmentShaderFilePath);

  spitfire::string_t sName;
  spitfire::string_t sFragmentShaderFilePath;
  bool bOn;
};

namespace util
{
  spitfire::math::cColour ChangeLuminance(const spitfire::math::cColour& colourRGB, float fLuminanceDifferenceMinusOneToPlusOne);
}

#endif // UTIL_H

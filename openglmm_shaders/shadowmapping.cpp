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
#include "shadowmapping.h"
#include "main.h"

// ** cShadowMapping

cShadowMapping::cShadowMapping() :
  pTextureDepthTexture(nullptr),
  pShaderShadowMap(nullptr),
  pShaderRenderToDepthTexture(nullptr)
{
}

void cShadowMapping::Init(opengl::cContext& context)
{
  pShaderRenderToDepthTexture = context.CreateShader(TEXT("shaders/depthrendertotexture.vert"), TEXT("shaders/depthrendertotexture.frag"));
  assert(pShaderRenderToDepthTexture != nullptr);
  pShaderShadowMap = context.CreateShader(TEXT("shaders/shadowmapping.vert"), TEXT("shaders/shadowmapping.frag"));
  assert(pShaderShadowMap != nullptr);
  pTextureDepthTexture = context.CreateTextureFrameBufferObjectDepthShadowOnly(1024, 1024);
  assert(pTextureDepthTexture != nullptr);
}

void cShadowMapping::Destroy(opengl::cContext& context)
{
  if (pShaderRenderToDepthTexture != nullptr) {
    context.DestroyShader(pShaderRenderToDepthTexture);
    pShaderRenderToDepthTexture = nullptr;
  }
  if (pShaderShadowMap != nullptr) {
    context.DestroyShader(pShaderShadowMap);
    pShaderShadowMap = nullptr;
  }
  if (pTextureDepthTexture != nullptr) {
    context.DestroyTextureFrameBufferObject(pTextureDepthTexture);
    pTextureDepthTexture = nullptr;
  }
}

void cShadowMapping::BeginRenderToShadowMap(opengl::cContext& context)
{
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(*pTextureDepthTexture);

  glDisable(GL_CULL_FACE);

  context.BindShader(*pShaderRenderToDepthTexture);
}

void cShadowMapping::EndRenderToShadowMap(opengl::cContext& context)
{
  glEnable(GL_CULL_FACE);

  context.UnBindShader(*pShaderRenderToDepthTexture);

  context.EndRenderToTexture(*pTextureDepthTexture);
}

spitfire::math::cVec3 lightInvDir;
spitfire::math::cMat4 matDepthMVP;
spitfire::math::cMat4 matDepthBiasMVP;

void cShadowMapping::RenderObjectToShadowMapSetMatrices(opengl::cContext& context, const spitfire::math::cVec3& lightPosition, const spitfire::math::cVec3& lightDirection, const spitfire::math::cVec3& objectPosition, const spitfire::math::cQuaternion& objectRotation)
{
  // Compute the MVP matrix from the light's point of view
  //glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
  //glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  // or, for spot light :
  //glm::vec3 lightPos(5, 20, 20);
  //glm::mat4 depthProjectionMatrix = glm::perspective<float>(45.0f, 1.0f, 2.0f, 50.0f);
  //glm::mat4 depthViewMatrix = glm::lookAt(lightPos, lightPos-lightInvDir, glm::vec3(0,1,0));

  lightInvDir = -lightDirection;

#if 0
  spitfire::math::cMat4 matProjection;
  matProjection.SetOrtho(-10, 10, -10, 10, -10, 20);

  spitfire::math::cMat4 matView;
  matView.LookAt(lightInvDir, spitfire::math::v3Zero, spitfire::math::v3Up);

  const spitfire::math::cMat4 matModel; // Identity matrix
  matDepthMVP = matProjection * matView * matModel;

  context.SetShaderConstant("matModelViewProjection", matDepthMVP);
#else
  const spitfire::math::cMat4 matProjection = context.CalculateProjectionMatrix();

  cFreeLookCamera camera;
  ASSERT(lightDirection.GetMagnitude() >= 0.0f);
  camera.LookAt(lightPosition, lightPosition - lightDirection, spitfire::math::v3Up);

  const spitfire::math::cMat4 matView = camera.CalculateViewMatrix();

  spitfire::math::cMat4 matObjectTranslation;
  matObjectTranslation.SetTranslation(objectPosition);

  spitfire::math::cMat4 matObjectRotation;
  matObjectRotation.SetRotation(objectRotation);

  const spitfire::math::cMat4 matModel = matObjectTranslation * matObjectRotation;

  context.SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matModel);
#endif
}

opengl::cTextureFrameBufferObject& cShadowMapping::GetShadowMapTexture()
{
  ASSERT(pTextureDepthTexture != nullptr);
  return *pTextureDepthTexture;
}

opengl::cShader& cShadowMapping::GetRenderToShadowMapShader()
{
  ASSERT(pShaderRenderToDepthTexture != nullptr);
  return *pShaderRenderToDepthTexture;
}

opengl::cShader& cShadowMapping::GetShadowMapShader()
{
  ASSERT(pShaderShadowMap != nullptr);
  return *pShaderShadowMap;
}

spitfire::math::cMat4 cShadowMapping::GetDepthMVP() const
{
  return matDepthMVP;
}

spitfire::math::cMat4 cShadowMapping::GetDepthBiasMVP() const
{
  const spitfire::math::cMat4 matBias(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
  );

  const spitfire::math::cMat4 depthBiasMVP = matBias * matDepthMVP;

  return depthBiasMVP;
}

spitfire::math::cVec3 cShadowMapping::GetLightInvDirectionWorldSpace() const
{
  return lightInvDir;
}

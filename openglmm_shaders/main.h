#ifndef MAIN_H
#define MAIN_H

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
#include "dofbokeh.h"
#include "hdr.h"
#include "shadowmapping.h"
#include "util.h"

// Test various lights, unfortunately it is a very large model which takes ages to load, so it is disabled by default
//#define BUILD_LARGE_STATUE_MODEL

struct KeyBoolPair {
  KeyBoolPair(unsigned int _key) : key(_key), bDown(false) {}

  void Process(unsigned int _key, bool _bDown) { if (_key == key) bDown = _bDown; }

  unsigned int key;
  bool bDown;
};

// ** cApplication

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();

  friend class cDOFBokeh;
  friend class cHDR;
  friend class cShadowMapping;

  bool Create();
  void Destroy();

  void Run();

  opengl::cResolution GetResolution() const;

protected:
  // Called from cHDR
  void CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fVBOWidth, float_t fVBOHeight, float_t fTextureWidth, float_t fTextureHeight);
  void RenderScreenRectangle(opengl::cTexture& texture, opengl::cShader& shader);
  void RenderScreenRectangle(opengl::cTexture& texture, opengl::cShader& shader, opengl::cStaticVertexBufferObject& staticVertexBufferObject);
  void RenderScreenRectangleShaderAndTextureAlreadySet(opengl::cStaticVertexBufferObject& staticVertexBufferObject);
  void RenderScreenRectangleShaderAndTextureAlreadySet();

private:
  void CreateShaders();
  void DestroyShaders();

  void CreateText();
  void CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fSizeMeters, float fTextureWidthWorldSpaceMeters);
  void CreateCube(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateBox(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateSphere(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fRadius);
  void CreateTeapot(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateGear(opengl::cStaticVertexBufferObject& vbo);

  void CreateLightBillboard();
  void CreateParticleSystem(opengl::cStaticVertexBufferObject& vbo);
  void CreateTestImage(opengl::cStaticVertexBufferObject& vbo, size_t nTextureWidth, size_t nTextureHeight);

  void CreateNormalMappedCube();

  void CreateTeapotVBO();
#ifdef BUILD_LARGE_STATUE_MODEL
  void CreateStatueVBO();
#endif
  void CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight);
  void CreateScreenHalfRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight);
  void CreateGuiRectangle(opengl::cStaticVertexBufferObject& staticVertexBufferObject, size_t nTextureWidth, size_t nTextureHeight);

  void RenderScreenRectangleDepthTexture(float x, float y, opengl::cStaticVertexBufferObject& vbo, opengl::cTextureFrameBufferObject& texture, opengl::cShader& shader);
  void RenderScreenRectangle(float x, float y, opengl::cStaticVertexBufferObject& vbo, opengl::cTexture& texture, opengl::cShader& shader);

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  std::vector<std::string> GetInputDescription() const;

  size_t GetActiveSimplePostRenderShadersCount() const;

  bool IsColourBlindSimplePostRenderShaderEnabled() const;
  size_t GetColourBlindModeDefineValue() const;

  bool bReloadShaders;
  bool bUpdateShaderConstants;

  bool bIsMovingForward;
  bool bIsMovingLeft;
  bool bIsMovingRight;
  bool bIsMovingBackward;

  KeyBoolPair moveLightForward;
  KeyBoolPair moveLightBack;
  KeyBoolPair moveLightLeft;
  KeyBoolPair moveLightRight;

  bool bIsCameraAtLightSource; // Whether the camera is moving around as normal or looking at the scene from the light's point of view

  bool bIsFocalLengthIncrease;
  bool bIsFocalLengthDecrease;
  bool bIsFStopIncrease;
  bool bIsFStopDecrease;

  bool bIsDirectionalLightOn;
  bool bIsPointLightOn;
  bool bIsSpotLightOn;
  spitfire::math::cVec3 lightPointPosition;

  bool bIsRotating;
  bool bIsWireframe;

  bool bIsDOFBokeh;

  bool bIsHDR;
  bool bIsSplitScreenSimplePostEffectShaders; // Tells us whether to split the screen down the middle when a simple post effect shader is active

  bool bIsDone;

  opengl::cSystem system;

  opengl::cResolution resolution;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  cFreeLookCamera camera;

  opengl::cFont* pFont;
  opengl::cStaticVertexBufferObject textVBO;

  opengl::cTextureFrameBufferObject* pTextureFrameBufferObjectTeapot;
  opengl::cTextureFrameBufferObject* pTextureFrameBufferObjectScreenColourAndDepth[2];
  opengl::cTextureFrameBufferObject* pTextureFrameBufferObjectScreenDepth;

  opengl::cTexture* pTextureDiffuse;
  opengl::cTexture* pTextureFelt;
  opengl::cTexture* pTextureLightMap;
  opengl::cTexture* pTextureDetail;
  opengl::cTextureCubeMap* pTextureCubeMap;
  opengl::cTexture* pTextureMarble;

  opengl::cTexture* pTextureMetalDiffuse;
  opengl::cTexture* pTextureMetalSpecular;

  opengl::cTexture* pTextureNormalMapDiffuse;
  opengl::cTexture* pTextureNormalMapSpecular;
  opengl::cTexture* pTextureNormalMapNormal;
  opengl::cTexture* pTextureNormalMapHeight;

  opengl::cTexture* pTextureFlare;

  opengl::cShader* pShaderColour;
  opengl::cShader* pShaderCubeMap;
  opengl::cShader* pShaderCarPaint;
  opengl::cShader* pShaderGlass;
  opengl::cShader* pShaderSilhouette;
  opengl::cShader* pShaderCelShaded;
  opengl::cShader* pShaderSmoke;
  opengl::cShader* pShaderFire;
  opengl::cShader* pShaderLights;
  opengl::cShader* pShaderLambert;
  opengl::cShader* pShaderPassThrough;
  opengl::cShader* pShaderScreenRect;
  opengl::cShader* pShaderScreenRectDepthShadow;
  opengl::cShader* pShaderScreenRectColourAndDepth;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectPlaneFloor;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectLargeTeapot;
#ifdef BUILD_LARGE_STATUE_MODEL
  opengl::cStaticVertexBufferObject staticVertexBufferObjectStatue;
#endif
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectScreen;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectHalfScreen;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectTeapot;


  opengl::cShader* pShaderCrate;
  opengl::cShader* pShaderFog;
  opengl::cShader* pShaderMetal;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectPlane0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectCube0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectBox0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectSphere0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectTeapot0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectGear0;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectPlane2;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectCube2;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectBox2;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectSphere2;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectTeapot2;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectPlane3;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectCube3;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectBox3;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectSphere3;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectTeapot3;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectPointLight;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectSpotLight;

  cShaderVBOPair parallaxNormalMap;

  cVBOShaderTexturePair light;
  cTextureVBOPair smoke;
  cTextureVBOPair fire;

  std::vector<cTextureVBOPair*> testImages;

  cDOFBokeh dofBokeh;
  cHDR hdr;
  cShadowMapping shadowMapping;

  std::vector<cSimplePostRenderShader> simplePostRenderShaders;
  bool bSimplePostRenderDirty;
  opengl::cShader* pShaderScreenRectSimplePostRender;

  enum class COLOUR_BLIND_MODE {
    PROTANOPIA,
    DEUTERANOPIA,
    TRITANOPIA
  };
  COLOUR_BLIND_MODE colourBlindMode;


  opengl::cStaticVertexBufferObject staticVertexBufferObjectGuiRectangle;
};

#endif // MAIN_H

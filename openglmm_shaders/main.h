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
#include <breathe/physics/verlet.h>
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
#include "anamorphic_lens_flare.h"
#include "dofbokeh.h"
#include "hdr.h"
#include "heathaze.h"
#include "lensflaredirt.h"
#include "shadowmapping.h"
#include "tronglow.h"
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

  friend class cGaussianBlur;
  friend class cTronGlow;
  friend class cHeatHaze;
  friend class cAnamorphicLensFlare;
  friend class cDOFBokeh;
  friend class cHDR;
  friend class cLensFlareDirt;
  friend class cShadowMapping;

  bool Create();
  void Destroy();

  void Run();

  opengl::cResolution GetResolution() const;

protected:
  // Called from cTronGlow, cHeatHaze, cHDR and cLensFlareDirt
  void CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fVBOWidth, float_t fVBOHeight, float_t fTextureWidth, float_t fTextureHeight);
  void RenderScreenRectangle(opengl::cTexture& texture, opengl::cShader& shader);
  void RenderScreenRectangle(opengl::cTexture& texture, opengl::cShader& shader, opengl::cStaticVertexBufferObject& staticVertexBufferObject);
  void RenderScreenRectangleShaderAndTextureAlreadySet(opengl::cStaticVertexBufferObject& staticVertexBufferObject);
  void RenderScreenRectangleShaderAndTextureAlreadySet();

private:
  void CreateShaders();
  void DestroyShaders();

  void CreateText();
  void CreateSquare(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fSizeMeters, float fTextureWidthWorldSpaceMeters);
  void CreateCube(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateBox(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateSphere(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fRadius);
  void CreateTeapot(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateGear(opengl::cStaticVertexBufferObject& vbo);

  void CreateBoxWithColourGradient(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fWidthMeters, float fDepthMeters, float fHeightMeters, const spitfire::math::cColour4& colourBottom, const spitfire::math::cColour4& colourTop);
  void CreateCylinderWithColourGradient(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fRadiusMeters, float fHeightMeters, const spitfire::math::cColour4& colourBottom, const spitfire::math::cColour4& colourTop);

  void CreateLightBillboard();
  void CreateParticleSystem(opengl::cStaticVertexBufferObject& vbo);
  void CreateTestImage(opengl::cStaticVertexBufferObject& vbo, size_t nTextureWidth, size_t nTextureHeight);

  void CreateNormalMappedCube();

  void CreateTeapotVBO();
#ifdef BUILD_LARGE_STATUE_MODEL
  void CreateStatueVBO();
#endif
  void CreateScreenRectVariableTextureSizeVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight);
  void CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight);
  void CreateScreenHalfRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight);
  void CreateGuiRectangle(opengl::cStaticVertexBufferObject& staticVertexBufferObject, size_t nTextureWidth, size_t nTextureHeight);

  void InitWavingFlags();
  void InitWavingFlag(breathe::physics::verlet::cGroup& group, float fWidthMeters, float fHeightMeters, size_t points_horizontal, size_t points_vertical);

  void UpdateWind();
  void UpdateFlags();

  void RenderScreenRectangleDepthTexture(float x, float y, opengl::cStaticVertexBufferObject& vbo, const opengl::cTextureFrameBufferObject& texture, opengl::cShader& shader);
  void RenderScreenRectangle(float x, float y, opengl::cStaticVertexBufferObject& vbo, const opengl::cTexture& texture, opengl::cShader& shader);
  void RenderDebugScreenRectangleVariableSize(float x, float y, const opengl::cTexture& texture);

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  std::vector<std::string> GetInputDescription() const;

  size_t GetActiveSimplePostRenderShadersCount() const;

  bool IsColourBlindSimplePostRenderShaderEnabled() const;
  size_t GetColourBlindModeDefineValue() const;

  float fFPS;

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

  bool bIsPhysicsRunning;
  bool bIsWireframe;

  enum class LENS_FLARE {
    NONE,
    DIRT, // A traditional lens flare with dirty screen effect
    ANAMORPHIC, // An anamorphic lens flare (Hi JJ Abrams!)
  };

  bool bIsTronGlow;
  bool bIsHeatHaze;
  bool bIsDOFBokeh;
  bool bIsHDR;
  bool bIsToneMapping;
  LENS_FLARE lensFlare;
  bool bDebugShowFlareOnly;
  bool bIsSplitScreenSimplePostEffectShaders; // Tells us whether to split the screen down the middle when a simple post effect shader is active

  bool bIsDone;

  opengl::cSystem system;

  opengl::cResolution resolution;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  cFreeLookCamera camera;

  opengl::cFont font;
  opengl::cStaticVertexBufferObject textVBO;

  opengl::cTextureFrameBufferObject textureFrameBufferObjectTeapot;
  opengl::cTextureFrameBufferObject textureFrameBufferObjectScreenColourAndDepth[4];
  opengl::cTextureFrameBufferObject textureFrameBufferObjectScreenDepth;

  opengl::cTexture textureDiffuse;
  opengl::cTexture textureFelt;
  opengl::cTexture textureLightMap;
  opengl::cTexture textureDetail;
  opengl::cTextureCubeMap textureCubeMap;
  opengl::cTextureCubeMap textureCarCubeMap;
  opengl::cTexture textureMarble;

  opengl::cTexture textureStainedGlass;
  opengl::cTexture textureStainedGlassNormalMap;
  opengl::cTexture textureStainedGlassGlossMap;

  opengl::cTexture textureStainedGlass2;
  opengl::cTexture textureStainedGlass2NormalMap;
  opengl::cTexture textureStainedGlass2GlossMap;

  opengl::cTexture textureMetalDiffuse;
  opengl::cTexture textureMetalSpecular;

  opengl::cTexture textureCarNormalMap;
  opengl::cTexture textureCarMicroFlakeNormalMap;

  opengl::cTexture textureSciFi;
  opengl::cTexture textureSciFiSpecular;
  opengl::cTexture textureSciFiNormalMap;
  opengl::cTexture textureSciFiHeightMap;
  opengl::cTexture textureSciFiGlowMap;

  opengl::cTexture textureCircuitTree;
  opengl::cTexture textureCircuitTreeGlowMap;

  opengl::cTexture textureNormalMapDiffuse;
  opengl::cTexture textureNormalMapSpecular;
  opengl::cTexture textureNormalMapNormal;
  opengl::cTexture textureNormalMapHeight;

  opengl::cTexture textureFlare;

  opengl::cShader shaderColour;
  opengl::cShader shaderCubeMap;
  opengl::cShader shaderBRDF;
  opengl::cShader shaderCarPaint;
  opengl::cShader shaderGlass;
  opengl::cShader shaderStainedGlass;
  opengl::cShader shaderSilhouette;
  opengl::cShader shaderCelShaded;
  opengl::cShader shaderSmoke;
  opengl::cShader shaderFire;
  opengl::cShader shaderLights;
  opengl::cShader shaderLambert;
  opengl::cShader shaderHemisphereLighting;
  opengl::cShader shaderRimLit;
  opengl::cShader shaderPassThrough;
  opengl::cShader shaderPassThroughNonRect;
  opengl::cShader shaderScreen1D;
  opengl::cShader shaderScreen2D;
  opengl::cShader shaderScreenRectVariableTextureSize;
  opengl::cShader shaderScreenRect;
  opengl::cShader shaderScreenRectDepthShadow;
  opengl::cShader shaderScreenRectColourAndDepth;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectPlaneFloor;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectLargeTeapot;
#ifdef BUILD_LARGE_STATUE_MODEL
  opengl::cStaticVertexBufferObject staticVertexBufferObjectStatue;
#endif
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectScreen;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectHalfScreen;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectTeapot;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreen2DTeapot;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectDebugVariableTextureSize;


  opengl::cShader shaderCrate;
  opengl::cShader shaderFog;
  opengl::cShader shaderMetal;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectPlane0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectCube0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectBox0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectSphere0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectTeapot0;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectGear0;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectSquare1;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectCube1;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectTeapot1;

  opengl::cStaticVertexBufferObject staticVertexBufferObjectBox1WithColours;
  opengl::cStaticVertexBufferObject staticVertexBufferObjectCylinder1WithColours;

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


  breathe::physics::verlet::cWorld physicsWorld;
  breathe::physics::verlet::cGroup wavingFlagPhysics[3];
  cTextureVBOPair wavingFlagRendering[3];


  cTronGlow tronGlow;
  cHeatHaze heatHaze;
  cDOFBokeh dofBokeh;
  cHDR hdr;
  cLensFlareDirt lensFlareDirt;
  cAnamorphicLensFlare lensFlareAnamorphic;
  cShadowMapping shadowMapping;

  std::vector<cSimplePostRenderShader> simplePostRenderShaders;
  bool bSimplePostRenderDirty;
  opengl::cShader shaderScreenRectSimplePostRender;

  enum class COLOUR_BLIND_MODE {
    PROTANOPIA,
    DEUTERANOPIA,
    TRITANOPIA
  };
  COLOUR_BLIND_MODE colourBlindMode;


  opengl::cStaticVertexBufferObject staticVertexBufferObjectGuiRectangle;
};

#endif // MAIN_H

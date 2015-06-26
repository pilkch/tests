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

// TODO: GET A HEBE STATUE MODEL

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

// Test various lights, unfortunately it is a very large model which takes ages to load, so it is disabled by default
//#define BUILD_LARGE_STATUE_MODEL

class cFreeLookCamera
{
public:
  cFreeLookCamera();

  spitfire::math::cVec3 GetPosition() const;
  void SetPosition(const spitfire::math::cVec3& position);
  void SetRotation(const spitfire::math::cQuaternion& rotation);

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
  fRotationRight -= xrmod;
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

  const spitfire::math::cQuaternion rotation = GetRotation();
  
  return ((-rotation).GetMatrix() * matTranslation);
}


class cTextureVBOPair
{
public:
  cTextureVBOPair();

  opengl::cTexture* pTexture;
  opengl::cStaticVertexBufferObject vbo;
};

cTextureVBOPair::cTextureVBOPair() :
  pTexture(nullptr)
{
}

class cShaderVBOPair
{
public:
  cShaderVBOPair();

  opengl::cShader* pShader;
  opengl::cStaticVertexBufferObject vbo;
};

cShaderVBOPair::cShaderVBOPair() :
  pShader(nullptr)
{
}


class cSimplePostRenderShader
{
public:
  cSimplePostRenderShader(const spitfire::string_t sName, const spitfire::string_t& sFragmentShaderFilePath);

  spitfire::string_t sName;
  spitfire::string_t sFragmentShaderFilePath;
  bool bOn;
};

cSimplePostRenderShader::cSimplePostRenderShader(const spitfire::string_t _sName, const spitfire::string_t& _sFragmentShaderFilePath) :
  sName(_sName),
  sFragmentShaderFilePath(_sFragmentShaderFilePath),
  bOn(false)
{
}

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();

  bool Create();
  void Destroy();

  void Run();

private:
  void CreateShaders();
  void DestroyShaders();

  void CreateText();
  void CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateCube(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateBox(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateSphere(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fRadius);
  void CreateTeapot(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates);
  void CreateGear(opengl::cStaticVertexBufferObject& vbo);
  
  void CreateTestImage(opengl::cStaticVertexBufferObject& vbo, size_t nTextureWidth, size_t nTextureHeight);

  void CreateNormalMappedCube();

  void CreateTeapotVBO();
  #ifdef BUILD_LARGE_STATUE_MODEL
  void CreateStatueVBO();
  #endif
  void CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight);
  void CreateScreenHalfRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight);
  void CreateGuiRectangle(opengl::cStaticVertexBufferObject& staticVertexBufferObject, size_t nTextureWidth, size_t nTextureHeight);
  
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

  bool bIsDirectionalLightOn;
  bool bIsPointLightOn;
  bool bIsSpotLightOn;

  bool bIsRotating;
  bool bIsWireframe;
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
  opengl::cTextureFrameBufferObject* pTextureFrameBufferObjectScreen;

  opengl::cTexture* pTextureDiffuse;
  opengl::cTexture* pTextureLightMap;
  opengl::cTexture* pTextureDetail;
  opengl::cTextureCubeMap* pTextureCubeMap;
  opengl::cTexture* pTextureMarble;
  opengl::cTexture* pTextureCelShaderColourMap;

  opengl::cTexture* pTextureNormalMapDiffuse;
  opengl::cTexture* pTextureNormalMapSpecular;
  opengl::cTexture* pTextureNormalMapNormal;
  opengl::cTexture* pTextureNormalMapHeight;

  opengl::cShader* pShaderCubeMap;
  opengl::cShader* pShaderCarPaint;
  opengl::cShader* pShaderSilhouette;
  opengl::cShader* pShaderCelShaded;
  opengl::cShader* pShaderLights;
  opengl::cShader* pShaderLambert;
  opengl::cShader* pShaderPassThrough;
  opengl::cShader* pShaderScreenRect;

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

  std::vector<cTextureVBOPair*> testImages;

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

cApplication::cApplication() :
  bReloadShaders(false),
  bUpdateShaderConstants(true),

  bIsMovingForward(false),
  bIsMovingLeft(false),
  bIsMovingRight(false),
  bIsMovingBackward(false),

  bIsDirectionalLightOn(true),
  bIsPointLightOn(true),
  bIsSpotLightOn(true),

  bIsRotating(true),
  bIsWireframe(false),
  bIsSplitScreenSimplePostEffectShaders(true),

  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  pFont(nullptr),

  pTextureFrameBufferObjectTeapot(nullptr),
  pTextureFrameBufferObjectScreen(nullptr),

  pTextureDiffuse(nullptr),
  pTextureLightMap(nullptr),
  pTextureDetail(nullptr),
  pTextureCubeMap(nullptr),
  pTextureMarble(nullptr),
  pTextureCelShaderColourMap(nullptr),

  pTextureNormalMapDiffuse(nullptr),
  pTextureNormalMapSpecular(nullptr),
  pTextureNormalMapNormal(nullptr),
  pTextureNormalMapHeight(nullptr),

  pShaderCubeMap(nullptr),
  pShaderCarPaint(nullptr),
  pShaderSilhouette(nullptr),
  pShaderCelShaded(nullptr),
  pShaderLambert(nullptr),
  pShaderLights(nullptr),
  pShaderPassThrough(nullptr),
  pShaderScreenRect(nullptr),

  pShaderCrate(nullptr),
  pShaderFog(nullptr),
  pShaderMetal(nullptr),

  bSimplePostRenderDirty(false),
  pShaderScreenRectSimplePostRender(nullptr),

  colourBlindMode(COLOUR_BLIND_MODE::PROTANOPIA)
{
  // Set our main thread
  spitfire::util::SetMainThread();

  // Set up our time variables
  spitfire::util::TimeInit();
}

void cApplication::CreateText()
{
  assert(pFont != nullptr);

  // Destroy any existing VBO
  pContext->DestroyStaticVertexBufferObject(textVBO);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v2_c4_t2 builder(*pGeometryDataPtr);

  std::list<spitfire::string_t> lines;
  lines.push_back(spitfire::string_t(TEXT("Rotating: ")) + (bIsRotating ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Wireframe: ")) + (bIsWireframe ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Direction light: ")) + (bIsDirectionalLightOn ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Point light: ")) + (bIsPointLightOn ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Spotlight: ")) + (bIsSpotLightOn ? TEXT("On") : TEXT("Off")));
  lines.push_back(TEXT(""));

  // Post render shaders
  if (GetActiveSimplePostRenderShadersCount() == 0) {
    lines.push_back(TEXT("Post render effects: "));
    lines.push_back(TEXT("None"));
  } else {
    lines.push_back(spitfire::string_t(TEXT("Split post render effects: ")) + (bIsSplitScreenSimplePostEffectShaders ? TEXT("On") : TEXT("Off")));
    lines.push_back(TEXT("Post render effects: "));
    // Print the name for each post render shader that is turned on
    const size_t n = simplePostRenderShaders.size();
    for (size_t i = 0; i < n; i++) {
      if (simplePostRenderShaders[i].bOn) lines.push_back(simplePostRenderShaders[i].sName);
    }

    // Print the colour blind mode
    if (IsColourBlindSimplePostRenderShaderEnabled()) {
      if (colourBlindMode == COLOUR_BLIND_MODE::PROTANOPIA) lines.push_back(TEXT("Protanopia"));
      else if (colourBlindMode == COLOUR_BLIND_MODE::DEUTERANOPIA) lines.push_back(TEXT("Deuteranopia"));
      else lines.push_back(TEXT("Tritanopia"));
    }
  }

  // Add our lines of text
  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);
  float y = 0.0f;
  std::list<spitfire::string_t>::const_iterator iter(lines.begin());
  const std::list<spitfire::string_t>::const_iterator iterEnd(lines.end());
  while (iter != iterEnd) {
    pFont->PushBack(builder, *iter, red, spitfire::math::cVec2(0.0f, y));
    y += 0.04f;

    iter++;
  }

  textVBO.SetData(pGeometryDataPtr);

  textVBO.Compile2D();
}

void cApplication::CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreatePlane(fWidth, fDepth, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateCube(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateCube(fWidth, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateBox(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;
  const float fHeight = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateBox(fWidth, fDepth, fHeight, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateSphere(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fRadius)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const size_t nSegments = 30;

  opengl::cGeometryBuilder builder;
  builder.CreateSphere(fRadius, nSegments, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateTeapot(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fRadius = 1.0f;
  const size_t nSegments = 20;

  opengl::cGeometryBuilder builder;
  builder.CreateTeapot(fRadius, nSegments, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateGear(opengl::cStaticVertexBufferObject& vbo)
{
}


class cGeometryBuilder_v3_n3_t2_tangent4
{
public:
  explicit cGeometryBuilder_v3_n3_t2_tangent4(opengl::cGeometryData& data);

  void PushBackQuad(
    const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2, const spitfire::math::cVec3& vertex3,
    const spitfire::math::cVec3& normal,
    const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2, const spitfire::math::cVec2& textureCoord3
  );

private:
  spitfire::math::cVec4 CalculateTangentVectorForTriangle(
    const spitfire::math::cVec3& pos1, const spitfire::math::cVec3& pos2, const spitfire::math::cVec3& pos3,
    const spitfire::math::cVec2& texCoord1, const spitfire::math::cVec2& texCoord2, const spitfire::math::cVec2& texCoord3,
    const spitfire::math::cVec3& normal
  ) const;

  void PushBackTriangle(
    const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2,
    const spitfire::math::cVec3& normal,
    const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2,
    const spitfire::math::cVec4& tangent
  );
  void PushBackVertex(const spitfire::math::cVec3& vertex, const spitfire::math::cVec3& normal, const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec4& tangent);

  opengl::cGeometryData& data;
};

// Given the 3 vertices (position and texture coordinates) of a triangle
// calculate and return the triangle's tangent vector.
// http://www.dhpoware.com/demos/gl3ParallaxNormalMapping.html
spitfire::math::cVec4 cGeometryBuilder_v3_n3_t2_tangent4::CalculateTangentVectorForTriangle(
  const spitfire::math::cVec3& pos1, const spitfire::math::cVec3& pos2, const spitfire::math::cVec3& pos3,
  const spitfire::math::cVec2& texCoord1, const spitfire::math::cVec2& texCoord2, const spitfire::math::cVec2& texCoord3,
  const spitfire::math::cVec3& normal
) const
{
  // Create 2 vectors in object space.
  //
  // edge1 is the vector from vertex positions pos1 to pos2.
  // edge2 is the vector from vertex positions pos1 to pos3.
  spitfire::math::cVec3 edge1(pos2 - pos1);
  spitfire::math::cVec3 edge2(pos3 - pos1);

  edge1.Normalise();
  edge2.Normalise();

  // Create 2 vectors in tangent (texture) space that point in the same
  // direction as edge1 and edge2 (in object space).
  //
  // texEdge1 is the vector from texture coordinates texCoord1 to texCoord2.
  // texEdge2 is the vector from texture coordinates texCoord1 to texCoord3.
  spitfire::math::cVec2 texEdge1(texCoord2 - texCoord1);
  spitfire::math::cVec2 texEdge2(texCoord3 - texCoord1);

  texEdge1.Normalise();
  texEdge2.Normalise();

  // These 2 sets of vectors form the following system of equations:
  //
  //  edge1 = (texEdge1.x * tangent) + (texEdge1.y * bitangent)
  //  edge2 = (texEdge2.x * tangent) + (texEdge2.y * bitangent)
  //
  // Using matrix notation this system looks like:
  //
  //  [ edge1 ]     [ texEdge1.x  texEdge1.y ]  [ tangent   ]
  //  [       ]  =  [                        ]  [           ]
  //  [ edge2 ]     [ texEdge2.x  texEdge2.y ]  [ bitangent ]
  //
  // The solution is:
  //
  //  [ tangent   ]        1     [ texEdge2.y  -texEdge1.y ]  [ edge1 ]
  //  [           ]  =  -------  [                         ]  [       ]
  //  [ bitangent ]      det A   [-texEdge2.x   texEdge1.x ]  [ edge2 ]
  //
  //  where:
  //        [ texEdge1.x  texEdge1.y ]
  //    A = [                        ]
  //        [ texEdge2.x  texEdge2.y ]
  //
  //    det A = (texEdge1.x * texEdge2.y) - (texEdge1.y * texEdge2.x)
  //
  // From this solution the tangent space basis vectors are:
  //
  //    tangent = (1 / det A) * ( texEdge2.y * edge1 - texEdge1.y * edge2)
  //  bitangent = (1 / det A) * (-texEdge2.x * edge1 + texEdge1.x * edge2)
  //     normal = cross(tangent, bitangent)

  float det = (texEdge1.x * texEdge2.y) - (texEdge1.y * texEdge2.x);

  spitfire::math::cVec3 t;
  spitfire::math::cVec3 b;
  if (spitfire::math::IsApproximatelyZero(det)) {
    t.Set(1.0f, 0.0f, 0.0f);
    b.Set(0.0f, 1.0f, 0.0f);
  } else {
    det = 1.0f / det;

    t.x = ((texEdge2.y * edge1.x) - (texEdge1.y * edge2.x)) * det;
    t.y = ((texEdge2.y * edge1.y) - (texEdge1.y * edge2.y)) * det;
    t.z = ((texEdge2.y * edge1.z) - (texEdge1.y * edge2.z)) * det;

    b.x = ((-texEdge2.x * edge1.x) + (texEdge1.x * edge2.x)) * det;
    b.y = ((-texEdge2.x * edge1.y) + (texEdge1.x * edge2.y)) * det;
    b.z = ((-texEdge2.x * edge1.z) + (texEdge1.x * edge2.z)) * det;

    t.Normalise();
    b.Normalise();
  }

  // Calculate the handedness of the local tangent space.
  // The bitangent vector is the cross product between the triangle face
  // normal vector and the calculated tangent vector. The resulting bitangent
  // vector should be the same as the bitangent vector calculated from the
  // set of linear equations above. If they point in different directions
  // then we need to invert the cross product calculated bitangent vector. We
  // store this scalar multiplier in the tangent vector's 'w' component so
  // that the correct bitangent vector can be generated in the normal mapping
  // shader's vertex shader.

  const spitfire::math::cVec3 bitangent = normal.CrossProduct(t);
  float handedness = (bitangent.DotProduct(b) < 0.0f) ? -1.0f : 1.0f;

  spitfire::math::cVec4 tangent;

  tangent.x = t.x;
  tangent.y = t.y;
  tangent.z = t.z;
  tangent.w = handedness;

  return tangent;
}

inline cGeometryBuilder_v3_n3_t2_tangent4::cGeometryBuilder_v3_n3_t2_tangent4(opengl::cGeometryData& _data) :
  data(_data)
{
  data.nVerticesPerPoint = 3;
  data.nNormalsPerPoint = 3;
  data.nTextureCoordinatesPerPoint = 2;
  data.nFloatUserData0PerPoint = 4;
}

void cGeometryBuilder_v3_n3_t2_tangent4::PushBackVertex(const spitfire::math::cVec3& vertex, const spitfire::math::cVec3& normal, const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec4& tangent)
{
  data.vertices.push_back(vertex.x);
  data.vertices.push_back(vertex.y);
  data.vertices.push_back(vertex.z);
  data.vertices.push_back(normal.x);
  data.vertices.push_back(normal.y);
  data.vertices.push_back(normal.z);
  data.vertices.push_back(textureCoord0.x);
  data.vertices.push_back(textureCoord0.y);
  data.vertices.push_back(tangent.x);
  data.vertices.push_back(tangent.y);
  data.vertices.push_back(tangent.z);
  data.vertices.push_back(tangent.w);
  data.nVertexCount++;
}

void cGeometryBuilder_v3_n3_t2_tangent4::PushBackTriangle(
  const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2,
  const spitfire::math::cVec3& normal,
  const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2,
  const spitfire::math::cVec4& tangent
)
{
  PushBackVertex(vertex0, normal, textureCoord0, tangent);
  PushBackVertex(vertex1, normal, textureCoord1, tangent);
  PushBackVertex(vertex2, normal, textureCoord2, tangent);
}

void cGeometryBuilder_v3_n3_t2_tangent4::PushBackQuad(
  const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2, const spitfire::math::cVec3& vertex3,
  const spitfire::math::cVec3& normal,
  const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2, const spitfire::math::cVec2& textureCoord3
)
{
  const spitfire::math::cVec4 tangent = CalculateTangentVectorForTriangle(
    vertex0, vertex1, vertex2,
    textureCoord0, textureCoord1, textureCoord2,
    normal
  );

  PushBackTriangle(vertex0, vertex1, vertex2, normal, textureCoord0, textureCoord1, textureCoord2, tangent);
  PushBackTriangle(vertex2, vertex3, vertex0, normal, textureCoord2, textureCoord3, textureCoord0, tangent);
}

void cApplication::CreateNormalMappedCube()
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  // Build a cube with an extra field for the tangent on each vertex
  cGeometryBuilder_v3_n3_t2_tangent4 builder(*pGeometryDataPtr);

  const float fWidth = 1.0f;

  const spitfire::math::cVec3 vMin(-fWidth * 0.5f, -fWidth * 0.5f, -fWidth * 0.5f);
  const spitfire::math::cVec3 vMax(fWidth * 0.5f, fWidth * 0.5f, fWidth * 0.5f);

  // Upper Square
  builder.PushBackQuad(
    spitfire::math::cVec3(vMin.x, vMin.y, vMax.z), spitfire::math::cVec3(vMax.x, vMin.y, vMax.z), spitfire::math::cVec3(vMax.x, vMax.y, vMax.z), spitfire::math::cVec3(vMin.x, vMax.y, vMax.z),
    spitfire::math::cVec3(0.0f, 0.0f, 1.0f),
    spitfire::math::cVec2(0.0f, 0.0f), spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f)
  );

  // Bottom Square
  builder.PushBackQuad(
    spitfire::math::cVec3(vMin.x, vMin.y, vMin.z), spitfire::math::cVec3(vMin.x, vMax.y, vMin.z), spitfire::math::cVec3(vMax.x, vMax.y, vMin.z), spitfire::math::cVec3(vMax.x, vMin.y, vMin.z),
    spitfire::math::cVec3(0.0f, 0.0f, -1.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  // Side Squares
  builder.PushBackQuad(
    spitfire::math::cVec3(vMin.x, vMax.y, vMin.z), spitfire::math::cVec3(vMin.x, vMax.y, vMax.z), spitfire::math::cVec3(vMax.x, vMax.y, vMax.z), spitfire::math::cVec3(vMax.x, vMax.y, vMin.z),
    spitfire::math::cVec3(0.0f, 1.0f, 0.0f),
    spitfire::math::cVec2(0.0f, 0.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(1.0f, 0.0f)
  );

  builder.PushBackQuad(
    spitfire::math::cVec3(vMax.x, vMin.y, vMin.z), spitfire::math::cVec3(vMax.x, vMin.y, vMax.z), spitfire::math::cVec3(vMin.x, vMin.y, vMax.z), spitfire::math::cVec3(vMin.x, vMin.y, vMin.z),
    spitfire::math::cVec3(0.0f, -1.0f, 0.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  // Front
  builder.PushBackQuad(
    spitfire::math::cVec3(vMax.x, vMax.y, vMin.z), spitfire::math::cVec3(vMax.x, vMax.y, vMax.z), spitfire::math::cVec3(vMax.x, vMin.y, vMax.z), spitfire::math::cVec3(vMax.x, vMin.y, vMin.z),
    spitfire::math::cVec3(0.0f, -1.0f, 0.0f),
    spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(0.0f, 0.0f), spitfire::math::cVec2(0.0f, 1.0f)
  );

  // Back
  builder.PushBackQuad(
    spitfire::math::cVec3(vMin.x, vMin.y, vMin.z), spitfire::math::cVec3(vMin.x, vMin.y, vMax.z), spitfire::math::cVec3(vMin.x, vMax.y, vMax.z), spitfire::math::cVec3(vMin.x, vMax.y, vMin.z),
    spitfire::math::cVec3(0.0f, 0.0f, 1.0f),
    spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(0.0f, 0.0f), spitfire::math::cVec2(0.0f, 1.0f)
  );

  parallaxNormalMap.vbo.SetData(pGeometryDataPtr);

  parallaxNormalMap.vbo.Compile();
}

void cApplication::CreateTeapotVBO()
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder builder;

  const float fRadius = 0.3f;
  const size_t nSegments = 20;
  builder.CreateTeapot(fRadius, nSegments, *pGeometryDataPtr, 1);

  staticVertexBufferObjectLargeTeapot.SetData(pGeometryDataPtr);

  staticVertexBufferObjectLargeTeapot.Compile();
}

#ifdef BUILD_LARGE_STATUE_MODEL
void cApplication::CreateStatueVBO()
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  breathe::render::model::cStaticModel model;

  breathe::render::model::cFileFormatOBJ loader;
  if (!loader.Load(TEXT("models/venus.obj"), model)) {
    LOG("Failed to load obj file");
    return;
  }

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

  // The obj file is giant so scale it down as well
  spitfire::math::cMat4 matScale;
  matScale.SetScale(spitfire::math::cVec3(0.1f, 0.1f, 0.1f));

  const spitfire::math::cMat4 matTransform = matScale;

  const size_t nMeshes = model.mesh.size();
  for (size_t iMesh = 0; iMesh < nMeshes; iMesh++) {
    const size_t nVertices = model.mesh[iMesh]->vertices.size() / 3;
    for (size_t iVertex = 0; iVertex < nVertices; iVertex++) {
      builder.PushBack(
        matTransform * spitfire::math::cVec3(model.mesh[iMesh]->vertices[(3 * iVertex)], model.mesh[iMesh]->vertices[(3 * iVertex) + 1], model.mesh[iMesh]->vertices[(3 * iVertex) + 2]),
        spitfire::math::cVec3(model.mesh[iMesh]->normals[(3 * iVertex)], model.mesh[iMesh]->normals[(3 * iVertex) + 1], model.mesh[iMesh]->normals[(3 * iVertex) + 2]),
        spitfire::math::cVec2(model.mesh[iMesh]->textureCoordinates[(2 * iVertex)], model.mesh[iMesh]->textureCoordinates[(2 * iVertex) + 1])
      );
    }
  }

  staticVertexBufferObjectStatue.SetData(pGeometryDataPtr);

  staticVertexBufferObjectStatue.Compile();
}
#endif

void cApplication::CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fTextureWidth = float(resolution.width);
  const float fTextureHeight = float(resolution.height);

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec2 vMin(-fHalfWidth, -fHalfHeight);
  const spitfire::math::cVec2 vMax(fHalfWidth, fHalfHeight);

  opengl::cGeometryBuilder_v2_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMax.y), spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMin.y), spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(fTextureWidth, fTextureHeight));

  staticVertexBufferObject.SetData(pGeometryDataPtr);

  staticVertexBufferObject.Compile2D();
}

void cApplication::CreateGuiRectangle(opengl::cStaticVertexBufferObject& vbo, size_t nTextureWidth, size_t nTextureHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fTextureWidth = float(nTextureWidth);
  const float fTextureHeight = float(nTextureHeight);

  const float fRatioWidthOverHeight = (fTextureWidth / fTextureHeight);

  const float fWidth = 1.0f;
  const float fHeight = fWidth / fRatioWidthOverHeight;

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec3 vMin(-fHalfWidth, 0.0f, -fHalfHeight);
  const spitfire::math::cVec3 vMax(fHalfWidth, 0.0f, fHalfHeight);
  const spitfire::math::cVec3 vNormal(0.0f, 0.0f, -1.0f);

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.z, 0.0f), vNormal, spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.z, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMax.z, 0.0f), vNormal, spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMin.z, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.z, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.z, 0.0f), vNormal, spitfire::math::cVec2(0.0f, fTextureHeight));

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateScreenHalfRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fHalfTextureWidth = 0.5f * float(resolution.width);
  const float fTextureHeight = float(resolution.height);

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec2 vMin(-fHalfWidth, -fHalfHeight);
  const spitfire::math::cVec2 vMax(fHalfWidth, fHalfHeight);

  opengl::cGeometryBuilder_v2_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(fHalfTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMax.y), spitfire::math::cVec2(fHalfTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMin.y), spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(fHalfTextureWidth, fTextureHeight));

  staticVertexBufferObject.SetData(pGeometryDataPtr);

  staticVertexBufferObject.Compile2D();
}

void cApplication::CreateTestImage(opengl::cStaticVertexBufferObject& vbo, size_t nTextureWidth, size_t nTextureHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fTextureWidth = float(nTextureWidth);
  const float fTextureHeight = float(nTextureHeight);

  const float fRatioWidthOverHeight = (fTextureWidth / fTextureHeight);

  const float fWidth = 1.0f;
  const float fHeight = fWidth / fRatioWidthOverHeight;

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec3 vMin(-fHalfWidth, -fHalfHeight, 0.0f);
  const spitfire::math::cVec3 vMax(fHalfWidth, fHalfHeight, 0.0f);
  const spitfire::math::cVec3 vNormal(0.0f, 0.0f, -1.0f);

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), vNormal, spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMin.y, 0.0f), vNormal, spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMax.y, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), vNormal, spitfire::math::cVec2(0.0f, 0.0f));

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

bool cApplication::Create()
{
  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 720) || (resolution.height < 480) || ((resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8) && (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8))) {
    LOGERROR("Current screen resolution is not adequate ", resolution.width, "x", resolution.height);
    return false;
  }

  #ifdef BUILD_DEBUG
  // Override the resolution
  opengl::cSystem::GetWindowedTestResolution16By9(resolution.width, resolution.height);
  #else
  resolution.width = 1000;
  resolution.height = 562;
  #endif
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  // Set our required resolution
  pWindow = system.CreateWindow(TEXT("OpenGLmm Shaders Test"), resolution, false);
  if (pWindow == nullptr) {
    LOGERROR("Window could not be created");
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    LOGERROR("Context could not be created");
    return false;
  }

  // Create our font
  pFont = pContext->CreateFont(TEXT("fonts/pricedown.ttf"), 32, TEXT("shaders/font.vert"), TEXT("shaders/font.frag"));
  assert(pFont != nullptr);
  assert(pFont->IsValid());

  CreateShaders();

  // Create our text VBO
  pContext->CreateStaticVertexBufferObject(textVBO);

  pTextureFrameBufferObjectTeapot = pContext->CreateTextureFrameBufferObject(resolution.width, resolution.height, opengl::PIXELFORMAT::R8G8B8A8);
  assert(pTextureFrameBufferObjectTeapot != nullptr);

  pTextureFrameBufferObjectScreen = pContext->CreateTextureFrameBufferObject(resolution.width, resolution.height, opengl::PIXELFORMAT::R8G8B8A8);
  assert(pTextureFrameBufferObjectScreen != nullptr);

  pTextureDiffuse = pContext->CreateTexture(TEXT("textures/diffuse.png"));
  assert(pTextureDiffuse != nullptr);
  pTextureLightMap = pContext->CreateTexture(TEXT("textures/lightmap.png"));
  assert(pTextureLightMap != nullptr);
  pTextureDetail = pContext->CreateTexture(TEXT("textures/detail.png"));
  assert(pTextureDetail != nullptr);

  pTextureCubeMap = pContext->CreateTextureCubeMap(
    TEXT("textures/skybox_positive_x.jpg"),
    TEXT("textures/skybox_negative_x.jpg"),
    TEXT("textures/skybox_positive_y.jpg"),
    TEXT("textures/skybox_negative_y.jpg"),
    TEXT("textures/skybox_positive_z.jpg"),
    TEXT("textures/skybox_negative_z.jpg")
  );
  assert(pTextureCubeMap != nullptr);

  pTextureMarble = pContext->CreateTexture(TEXT("textures/marble.png"));
  assert(pTextureMarble != nullptr);

  voodoo::cImage image;
  {
    // Create our image
    struct cColourRGB255 {
      uint8_t r;
      uint8_t g;
      uint8_t b;
    };
    const cColourRGB255 colours[] = {
      { 0, 0, 0 }, // Black
      { 124, 76, 0 }, // Black
      { 255, 159, 0 }, // Orange
      { 234, 218, 0 }, // Yellow
      { 255, 255, 0 }, // Light yellow
      { 255, 255, 255 }, // White
    };

    // TODO: Create a 4x4 image or even 4x1?
    const size_t width = 128;
    const size_t height = 128;
    const size_t nParts = countof(colours);
    const size_t partWidth = (width / nParts) * 3;
    std::array<uint8_t, width * height * 3> data;

    // Fill in the first line of pixels
    for (size_t part = 0; part < nParts; part++) {
      for (size_t index = (part * partWidth); index < ((part + 1) * partWidth); index += 3) {
        data[index] = colours[part].r;
        data[index + 1] = colours[part].g;
        data[index + 2] = colours[part].b;
      }
    }

    const size_t rowWidthBytes = width * 3;

    // Copy the first line of pixels over the other lines
    for (size_t y = 0; y < height; y++) {
      const size_t index = (y * rowWidthBytes);
      memcpy(&data[index], &data[0], rowWidthBytes);
    }

    image.CreateFromBuffer(data.data(), width, height, voodoo::PIXELFORMAT::R8G8B8);
  }
  pTextureCelShaderColourMap = pContext->CreateTextureFromImage(image);
  assert(pTextureCelShaderColourMap != nullptr);

  pTextureNormalMapDiffuse = pContext->CreateTexture(TEXT("textures/floor_tile_color_map.png"));
  assert(pTextureNormalMapDiffuse != nullptr);

  pTextureNormalMapSpecular = pContext->CreateTexture(TEXT("textures/floor_tile_gloss_map.png"));
  assert(pTextureNormalMapSpecular != nullptr);

  pTextureNormalMapNormal = pContext->CreateTexture(TEXT("textures/floor_tile_normal_map.png"));
  assert(pTextureNormalMapNormal != nullptr);

  pTextureNormalMapHeight = pContext->CreateTexture(TEXT("textures/floor_tile_height_map.png"));
  assert(pTextureNormalMapHeight != nullptr);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);
  CreateTeapotVBO();

  #ifdef BUILD_LARGE_STATUE_MODEL
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectStatue);
  CreateStatueVBO();
  #endif

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectScreen);
  CreateScreenRectVBO(staticVertexBufferObjectScreenRectScreen, 1.0f, 1.0f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);
  CreateGuiRectangle(staticVertexBufferObjectGuiRectangle, 1000.0f, 500.0f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectHalfScreen);
  CreateScreenHalfRectVBO(staticVertexBufferObjectScreenRectHalfScreen, 0.5f, 1.0f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectTeapot);
  CreateScreenRectVBO(staticVertexBufferObjectScreenRectTeapot, 0.25f, 0.25f);


  const spitfire::string_t sTestImagesPath = TEXT("textures/testimages/");
  for (spitfire::filesystem::cFolderIterator iter(sTestImagesPath); iter.IsValid(); iter.Next()) {
    if (iter.IsFile()) {
      cTextureVBOPair* pPair = new cTextureVBOPair;
      pPair->pTexture = pContext->CreateTexture(iter.GetFullPath());
      assert(pPair->pTexture != nullptr);
      pContext->CreateStaticVertexBufferObject(pPair->vbo);
      CreateTestImage(pPair->vbo, pPair->pTexture->GetWidth(), pPair->pTexture->GetHeight());
      testImages.push_back(pPair);
    }
  }


  const float fRadius = 1.0f;

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPlane0);
  CreatePlane(staticVertexBufferObjectPlane0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectCube0);
  CreateCube(staticVertexBufferObjectCube0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectBox0);
  CreateBox(staticVertexBufferObjectBox0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSphere0);
  CreateSphere(staticVertexBufferObjectSphere0, 0, fRadius);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
  CreateTeapot(staticVertexBufferObjectTeapot0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectGear0);
  CreateGear(staticVertexBufferObjectGear0);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPlane2);
  CreatePlane(staticVertexBufferObjectPlane2, 2);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectCube2);
  CreateCube(staticVertexBufferObjectCube2, 2);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectBox2);
  CreateBox(staticVertexBufferObjectBox2, 2);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSphere2);
  CreateSphere(staticVertexBufferObjectSphere2, 2, fRadius);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
  CreateTeapot(staticVertexBufferObjectTeapot2, 2);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPlane3);
  CreatePlane(staticVertexBufferObjectPlane3, 3);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectCube3);
  CreateCube(staticVertexBufferObjectCube3, 3);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectBox3);
  CreateBox(staticVertexBufferObjectBox3, 3);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSphere3);
  CreateSphere(staticVertexBufferObjectSphere3, 3, fRadius);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
  CreateTeapot(staticVertexBufferObjectTeapot3, 3);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPointLight);
  CreateSphere(staticVertexBufferObjectPointLight, 0, 0.3f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSpotLight);
  CreateSphere(staticVertexBufferObjectSpotLight, 0, 0.3f);

  pContext->CreateStaticVertexBufferObject(parallaxNormalMap.vbo);
  CreateNormalMappedCube();


  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Sepia"), TEXT("shaders/sepia.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Noir"), TEXT("shaders/noir.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Matrix"), TEXT("shaders/matrix.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Teal and Orange"), TEXT("shaders/tealandorange.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Colour blind simulation"), TEXT("shaders/colourblind.frag")));


  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  ASSERT(pContext != nullptr);

  pContext->DestroyStaticVertexBufferObject(parallaxNormalMap.vbo);

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSpotLight);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPointLight);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSphere3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectBox3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectCube3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPlane3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSphere2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectBox2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectCube2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPlane2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectGear0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSphere0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectBox0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectCube0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPlane0);

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectTeapot);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectHalfScreen);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectScreen);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);

  #ifdef BUILD_LARGE_STATUE_MODEL
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectStatue);
  #endif

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);


  if (pTextureNormalMapHeight != nullptr) {
    pContext->DestroyTexture(pTextureNormalMapHeight);
    pTextureNormalMapHeight = nullptr;
  }
  if (pTextureNormalMapNormal != nullptr) {
    pContext->DestroyTexture(pTextureNormalMapNormal);
    pTextureNormalMapNormal = nullptr;
  }
  if (pTextureNormalMapSpecular != nullptr) {
    pContext->DestroyTexture(pTextureNormalMapSpecular);
    pTextureNormalMapSpecular = nullptr;
  }
  if (pTextureNormalMapDiffuse != nullptr) {
    pContext->DestroyTexture(pTextureNormalMapDiffuse);
    pTextureNormalMapDiffuse = nullptr;
  }

  if (pTextureCelShaderColourMap != nullptr) {
    pContext->DestroyTexture(pTextureCelShaderColourMap);
    pTextureCelShaderColourMap = nullptr;
  }
  if (pTextureMarble != nullptr) {
    pContext->DestroyTexture(pTextureMarble);
    pTextureMarble = nullptr;
  }
  if (pTextureCubeMap != nullptr) {
    pContext->DestroyTextureCubeMap(pTextureCubeMap);
    pTextureCubeMap = nullptr;
  }
  if (pTextureDetail != nullptr) {
    pContext->DestroyTexture(pTextureDetail);
    pTextureDetail = nullptr;
  }
  if (pTextureLightMap != nullptr) {
    pContext->DestroyTexture(pTextureLightMap);
    pTextureLightMap = nullptr;
  }
  if (pTextureDiffuse != nullptr) {
    pContext->DestroyTexture(pTextureDiffuse);
    pTextureDiffuse = nullptr;
  }

  if (pTextureFrameBufferObjectTeapot != nullptr) {
    pContext->DestroyTextureFrameBufferObject(pTextureFrameBufferObjectTeapot);
    pTextureFrameBufferObjectTeapot = nullptr;
  }
  if (pTextureFrameBufferObjectScreen != nullptr) {
    pContext->DestroyTextureFrameBufferObject(pTextureFrameBufferObjectScreen);
    pTextureFrameBufferObjectScreen = nullptr;
  }


  const size_t n = testImages.size();
  for (size_t i = 0; i < n; i++) {
    if (testImages[i]->pTexture != nullptr) {
      pContext->DestroyTexture(testImages[i]->pTexture);
      testImages[i]->pTexture = nullptr;
    }

    pContext->DestroyStaticVertexBufferObject(testImages[i]->vbo);

    delete testImages[i];
  }

  testImages.clear();

  // Destroy our text VBO
  pContext->DestroyStaticVertexBufferObject(textVBO);

  // Destroy our font
  if (pFont != nullptr) {
    pContext->DestroyFont(pFont);
    pFont = nullptr;
  }

  DestroyShaders();

  if (pShaderScreenRectSimplePostRender != nullptr) {
    pContext->DestroyShader(pShaderScreenRectSimplePostRender);
    pShaderScreenRectSimplePostRender = nullptr;
  }

  pContext = nullptr;

  if (pWindow != nullptr) {
    system.DestroyWindow(pWindow);
    pWindow = nullptr;
  }
}

void cApplication::CreateShaders()
{
  pShaderCubeMap = pContext->CreateShader(TEXT("shaders/cubemap.vert"), TEXT("shaders/cubemap.frag"));
  assert(pShaderCubeMap != nullptr);

  pShaderCarPaint = pContext->CreateShader(TEXT("shaders/carpaint.vert"), TEXT("shaders/carpaint.frag"));
  assert(pShaderCarPaint != nullptr);

  pShaderSilhouette = pContext->CreateShader(TEXT("shaders/silhouette.vert"), TEXT("shaders/silhouette.frag"));
  assert(pShaderSilhouette != nullptr);

  pShaderCelShaded = pContext->CreateShader(TEXT("shaders/celshader.vert"), TEXT("shaders/celshader.frag"));
  assert(pShaderCelShaded != nullptr);

  pShaderLambert = pContext->CreateShader(TEXT("shaders/lambert.vert"), TEXT("shaders/lambert.frag"));
  assert(pShaderLambert != nullptr);

  pShaderLights = pContext->CreateShader(TEXT("shaders/lights.vert"), TEXT("shaders/lights.frag"));
  assert(pShaderLights != nullptr);

  parallaxNormalMap.pShader = pContext->CreateShader(TEXT("shaders/parallaxnormalmap.vert"), TEXT("shaders/parallaxnormalmap.frag"));
  assert(parallaxNormalMap.pShader != nullptr);

  pShaderPassThrough = pContext->CreateShader(TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(pShaderPassThrough != nullptr);

  pShaderScreenRect = pContext->CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/passthrough2d.frag"));
  assert(pShaderScreenRect != nullptr);

  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  assert(pShaderCrate != nullptr);

  pShaderFog = pContext->CreateShader(TEXT("shaders/fog.vert"), TEXT("shaders/fog.frag"));
  assert(pShaderFog != nullptr);

  pShaderMetal = pContext->CreateShader(TEXT("shaders/metal.vert"), TEXT("shaders/metal.frag"));
  assert(pShaderMetal != nullptr);
}

void cApplication::DestroyShaders()
{
  if (pShaderScreenRect != nullptr) {
    pContext->DestroyShader(pShaderScreenRect);
    pShaderScreenRect = nullptr;
  }
  if (pShaderPassThrough != nullptr) {
    pContext->DestroyShader(pShaderPassThrough);
    pShaderPassThrough = nullptr;
  }

  if (parallaxNormalMap.pShader != nullptr) {
    pContext->DestroyShader(parallaxNormalMap.pShader);
    parallaxNormalMap.pShader = nullptr;
  }

  if (pShaderLambert != nullptr) {
    pContext->DestroyShader(pShaderLambert);
    pShaderLambert = nullptr;
  }
  if (pShaderLights != nullptr) {
    pContext->DestroyShader(pShaderLights);
    pShaderLights = nullptr;
  }

  if (pShaderCelShaded != nullptr) {
      pContext->DestroyShader(pShaderCelShaded);
      pShaderCelShaded = nullptr;
    }
  if (pShaderSilhouette != nullptr) {
    pContext->DestroyShader(pShaderSilhouette);
    pShaderSilhouette = nullptr;
  }
  if (pShaderCarPaint != nullptr) {
    pContext->DestroyShader(pShaderCarPaint);
    pShaderCarPaint = nullptr;
  }
  if (pShaderCubeMap != nullptr) {
    pContext->DestroyShader(pShaderCubeMap);
    pShaderCubeMap = nullptr;
  }

  if (pShaderMetal != nullptr) {
    pContext->DestroyShader(pShaderMetal);
    pShaderMetal = nullptr;
  }
  if (pShaderFog != nullptr) {
    pContext->DestroyShader(pShaderFog);
    pShaderFog = nullptr;
  }
  if (pShaderCrate != nullptr) {
    pContext->DestroyShader(pShaderCrate);
    pShaderCrate = nullptr;
  }
}

void cApplication::RenderScreenRectangle(float x, float y, opengl::cStaticVertexBufferObject& vbo, opengl::cTexture& texture, opengl::cShader& shader)
{
  spitfire::math::cMat4 matModelView2D;
  matModelView2D.SetTranslation(x, y, 0.0f);

  pContext->BindTexture(0, texture);

  pContext->BindShader(shader);

  pContext->BindStaticVertexBufferObject2D(vbo);

  {
    pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

    pContext->DrawStaticVertexBufferObjectTriangles2D(vbo);
  }

  pContext->UnBindStaticVertexBufferObject2D(vbo);

  pContext->UnBindShader(shader);

  pContext->UnBindTexture(0, texture);
}

void cApplication::_OnWindowEvent(const opengl::cWindowEvent& event)
{
  LOG("");

  if (event.IsQuit()) {
    LOG("Quiting");
    bIsDone = true;
  }
}

void cApplication::_OnMouseEvent(const opengl::cMouseEvent& event)
{
  // These are a little too numerous to log every single one
  //LOG("");

  if (event.IsMouseMove()) {
    //LOG("Mouse move");

    if (fabs(event.GetX() - (pWindow->GetWidth() * 0.5f)) > 0.5f) {
      camera.RotateX(-0.08f * (event.GetX() - (pWindow->GetWidth() * 0.5f)));
    }

    if (fabs(event.GetY() - (pWindow->GetHeight() * 0.5f)) > 1.5f) {
      camera.RotateY(0.05f * (event.GetY() - (pWindow->GetHeight() * 0.5f)));
    }
  }
}

void cApplication::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  //LOG("");
  if (event.IsKeyDown()) {
    switch (event.GetKeyCode()) {
      case SDLK_ESCAPE: {
        LOG("Escape key pressed, quiting");
        bIsDone = true;
        break;
      }
      case SDLK_w: {
        bIsMovingForward = true;
        break;
      }
      case SDLK_a: {
        bIsMovingLeft = true;
        break;
      }
      case SDLK_s: {
        bIsMovingBackward = true;
        break;
      }
      case SDLK_d: {
        bIsMovingRight = true;
        break;
      }
      case SDLK_SPACE: {
        LOG("spacebar down");
        bIsRotating = false;
        break;
      }
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case SDLK_SPACE: {
        LOG("spacebar up");
        bIsRotating = true;
        break;
      }
      case SDLK_w: {
        bIsMovingForward = false;
        break;
      }
      case SDLK_a: {
        bIsMovingLeft = false;
        break;
      }
      case SDLK_s: {
        bIsMovingBackward = false;
        break;
      }
      case SDLK_d: {
        bIsMovingRight = false;
        break;
      }
      case SDLK_F5: {
        bReloadShaders = true;
        bSimplePostRenderDirty = true;
        bUpdateShaderConstants = true;
        break;
      }
      case SDLK_y: {
        simplePostRenderShaders[0].bOn = !simplePostRenderShaders[0].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_u: {
        simplePostRenderShaders[1].bOn = !simplePostRenderShaders[1].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_i: {
        simplePostRenderShaders[2].bOn = !simplePostRenderShaders[2].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_o: {
        simplePostRenderShaders[3].bOn = !simplePostRenderShaders[3].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_p: {
        simplePostRenderShaders[4].bOn = !simplePostRenderShaders[4].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_l: {
        if (IsColourBlindSimplePostRenderShaderEnabled()) {
          if (colourBlindMode == COLOUR_BLIND_MODE::PROTANOPIA) colourBlindMode = COLOUR_BLIND_MODE::DEUTERANOPIA;
          else if (colourBlindMode == COLOUR_BLIND_MODE::DEUTERANOPIA) colourBlindMode = COLOUR_BLIND_MODE::TRITANOPIA;
          else colourBlindMode = COLOUR_BLIND_MODE::PROTANOPIA;
          bSimplePostRenderDirty = true;
        }
        break;
      }
      case SDLK_1: {
        bIsWireframe = !bIsWireframe;
        break;
      }
      case SDLK_2: {
        bIsDirectionalLightOn = !bIsDirectionalLightOn;
        break;
      }
      case SDLK_3: {
        bIsPointLightOn = !bIsPointLightOn;
        break;
      }
      case SDLK_4: {
        bIsSpotLightOn = !bIsSpotLightOn;
        break;
      }
      case SDLK_5: {
        bIsSplitScreenSimplePostEffectShaders = !bIsSplitScreenSimplePostEffectShaders;
        break;
      }
    }
  }
}

std::vector<std::string> cApplication::GetInputDescription() const
{
  std::vector<std::string> description;
  description.push_back("W forward");
  description.push_back("A left");
  description.push_back("S backward");
  description.push_back("D right");
  description.push_back("Space pause rotation");
  description.push_back("F5 reload shaders");
  description.push_back("1 toggle wireframe");
  description.push_back("2 toggle directional light");
  description.push_back("3 toggle point light");
  description.push_back("4 toggle spot light");
  description.push_back("5 toggle split screen post render");
  description.push_back("Y toggle post render sepia");
  description.push_back("U toggle post render noir");
  description.push_back("I toggle post render matrix");
  description.push_back("O toggle post render teal and orange");
  description.push_back("P toggle post render colour blind simulation");
  description.push_back("L switch colour blind mode");
  description.push_back("Esc quit");

  return description;
}

size_t cApplication::GetActiveSimplePostRenderShadersCount() const
{
  size_t count = 0;

  const size_t n = simplePostRenderShaders.size();
  for (size_t i = 0; i < n; i++) {
    if (simplePostRenderShaders[i].bOn) count++;
  }

  return count;
}

bool cApplication::IsColourBlindSimplePostRenderShaderEnabled() const
{
  ASSERT(4 < simplePostRenderShaders.size());
  return simplePostRenderShaders[4].bOn;
}

size_t cApplication::GetColourBlindModeDefineValue() const
{
  if (colourBlindMode == COLOUR_BLIND_MODE::PROTANOPIA) return 0;
  else if (colourBlindMode == COLOUR_BLIND_MODE::DEUTERANOPIA) return 1;

  return 2;
}

void cApplication::Run()
{
  LOG("");

  assert(pContext != nullptr);
  assert(pContext->IsValid());
  assert(pFont != nullptr);
  assert(pFont->IsValid());
  assert(pTextureDiffuse != nullptr);
  assert(pTextureDiffuse->IsValid());
  assert(pTextureLightMap != nullptr);
  assert(pTextureLightMap->IsValid());
  assert(pTextureDetail != nullptr);
  assert(pTextureDetail->IsValid());
  assert(pTextureCubeMap != nullptr);
  assert(pTextureCubeMap->IsValid());
  assert(pTextureMarble != nullptr);
  assert(pTextureMarble->IsValid());
  assert(pTextureCelShaderColourMap != nullptr);
  assert(pTextureCelShaderColourMap->IsValid());
  assert(pTextureNormalMapDiffuse != nullptr);
  assert(pTextureNormalMapDiffuse->IsValid());
  assert(pTextureNormalMapSpecular != nullptr);
  assert(pTextureNormalMapSpecular->IsValid());
  assert(pTextureNormalMapHeight != nullptr);
  assert(pTextureNormalMapHeight->IsValid());
  assert(pTextureNormalMapNormal != nullptr);
  assert(pTextureNormalMapNormal->IsValid());
  assert(pShaderCubeMap != nullptr);
  assert(pShaderCubeMap->IsCompiledProgram());
  assert(pShaderCarPaint != nullptr);
  assert(pShaderCarPaint->IsCompiledProgram());
  assert(pShaderSilhouette != nullptr);
  assert(pShaderSilhouette->IsCompiledProgram());
  assert(pShaderCelShaded != nullptr);
  assert(pShaderCelShaded->IsCompiledProgram());
  assert(pShaderLambert != nullptr);
  assert(pShaderLambert->IsCompiledProgram());
  assert(pShaderLights != nullptr);
  assert(pShaderLights->IsCompiledProgram());
  assert(parallaxNormalMap.pShader != nullptr);
  assert(parallaxNormalMap.pShader->IsCompiledProgram());
  assert(pShaderPassThrough != nullptr);
  assert(pShaderPassThrough->IsCompiledProgram());
  assert(pShaderScreenRect != nullptr);
  assert(pShaderScreenRect->IsCompiledProgram());

  assert(staticVertexBufferObjectLargeTeapot.IsCompiled());
  #ifdef BUILD_LARGE_STATUE_MODEL
  assert(staticVertexBufferObjectStatue.IsCompiled());
  #endif
  assert(staticVertexBufferObjectScreenRectScreen.IsCompiled());
  assert(staticVertexBufferObjectScreenRectHalfScreen.IsCompiled());
  assert(staticVertexBufferObjectScreenRectTeapot.IsCompiled());


  assert(pShaderCrate != nullptr);
  assert(pShaderCrate->IsCompiledProgram());
  assert(pShaderFog != nullptr);
  assert(pShaderFog->IsCompiledProgram());
  assert(pShaderMetal != nullptr);
  assert(pShaderMetal->IsCompiledProgram());

  assert(staticVertexBufferObjectPlane0.IsCompiled());
  assert(staticVertexBufferObjectCube0.IsCompiled());
  assert(staticVertexBufferObjectBox0.IsCompiled());
  assert(staticVertexBufferObjectSphere0.IsCompiled());
  assert(staticVertexBufferObjectTeapot0.IsCompiled());
  //assert(staticVertexBufferObjectGear0.IsCompiled());

  assert(staticVertexBufferObjectPlane2.IsCompiled());
  assert(staticVertexBufferObjectCube2.IsCompiled());
  assert(staticVertexBufferObjectBox2.IsCompiled());
  assert(staticVertexBufferObjectSphere2.IsCompiled());
  assert(staticVertexBufferObjectTeapot2.IsCompiled());

  assert(staticVertexBufferObjectPlane3.IsCompiled());
  assert(staticVertexBufferObjectCube3.IsCompiled());
  assert(staticVertexBufferObjectBox3.IsCompiled());
  assert(staticVertexBufferObjectSphere3.IsCompiled());
  assert(staticVertexBufferObjectTeapot3.IsCompiled());

  assert(staticVertexBufferObjectPointLight.IsCompiled());
  assert(staticVertexBufferObjectSpotLight.IsCompiled());
  assert(parallaxNormalMap.vbo.IsCompiled());

  // Print the input instructions
  const std::vector<std::string> inputDescription = GetInputDescription();
  const size_t n = inputDescription.size();
  for (size_t i = 0; i < n; i++) LOG(inputDescription[i]);

  // Set up the camera
  camera.SetPosition(spitfire::math::cVec3(-6.5f, 2.5f, 7.0f));
  camera.RotateX(90.0f);
  camera.RotateY(45.0f);

  // Set up the translations for our objects
  const size_t columns = 5; // 5 types of objects
  const size_t rows = 5; // 5 types of materials

  const float fSpacingX = 0.007f * pContext->GetWidth() / float(rows);
  const float fSpacingZ = 0.03f * pContext->GetHeight() / float(columns);
  const float fLeft = -0.5f * float(columns - 1) * fSpacingX;

  spitfire::math::cVec3 positions[columns * rows];
  size_t i = 0;
  for (size_t y = 0; y < rows; y++) {
    for (size_t x = 0; x < columns; x++) {
      positions[i].Set(fLeft + (x * fSpacingX), 0.0f, (y * fSpacingZ));
      i++;
    }
  }

  spitfire::math::cMat4 matTranslationArray[columns * rows];
  for (size_t i = 0; i < columns * rows; i++) matTranslationArray[i].SetTranslation(positions[i]);


  spitfire::math::cQuaternion rotation;
  spitfire::math::cMat4 matRotation;

  float fAngleRadians = 0.0f;
  const float fRotationSpeed = 0.001f;

  spitfire::math::cMat4 matObjectRotation;

  // Cube mapped teapot
  const spitfire::math::cVec3 positionCubeMappedTeapot(-fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationCubeMappedTeapot;
  matTranslationCubeMappedTeapot.SetTranslation(positionCubeMappedTeapot);

  // Car paint teapot
  const spitfire::math::cVec3 positionCarPaintTeapot(-2.0f * fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationCarPaintTeapot;
  matTranslationCarPaintTeapot.SetTranslation(positionCarPaintTeapot);

  // Cel shaded teapot
  const spitfire::math::cVec3 positionCelShadedTeapot(-4.0f * fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationCelShadedTeapot;
  matTranslationCelShadedTeapot.SetTranslation(positionCelShadedTeapot);

  // Parallax normal mapping
  const spitfire::math::cVec3 parallaxNormalMapPosition(fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationParallaxNormalMap;
  matTranslationParallaxNormalMap.SetTranslation(parallaxNormalMapPosition);

  #ifdef BUILD_LARGE_STATUE_MODEL
  spitfire::math::cMat4 matTranslationStatue[9];
  i = 0;
  for (size_t y = 0; y < 3; y++) {
    for (size_t x = 0; x < 3; x++) {
      const spitfire::math::cVec3 position((-1.0f + float(x)) * fSpacingX, 0.0f, ((-2.0f - float(y)) * fSpacingZ));
      matTranslationStatue[i].SetTranslation(position);
      i++;
    }
  }
  #endif

  std::vector<spitfire::math::cMat4> matTranslationTestImages;
  {
    const size_t n = testImages.size();
    for (size_t i = 0; i < n; i++) {
      const spitfire::math::cVec3 position((-1.0f + float(i)) * fSpacingX, 0.0f, -5.0f * fSpacingZ);
      spitfire::math::cMat4 matTranslation;
      matTranslation.SetTranslation(position);
      matTranslationTestImages.push_back(matTranslation);
    }
  }


  spitfire::durationms_t T0 = spitfire::util::GetTimeMS();
  uint32_t Frames = 0;

  spitfire::durationms_t previousUpdateInputTime = spitfire::util::GetTimeMS();
  spitfire::durationms_t previousUpdateTime = spitfire::util::GetTimeMS();
  spitfire::durationms_t currentTime = spitfire::util::GetTimeMS();


  // Green directional light
  const spitfire::math::cVec3 lightDirectionalPosition(5.0f, 5.0f, 10.0f);
  const spitfire::math::cColour lightDirectionalAmbientColour(0.2f, 0.25f, 0.2f);
  const spitfire::math::cColour lightDirectionalDiffuseColour(0.6f, 0.8f, 0.6f);
  const spitfire::math::cColour lightDirectionalSpecularColour(0.0f, 1.0f, 0.0f);

  // Red point light
  const spitfire::math::cVec3 lightPointPosition(-5.0f, -5.0f, 1.0f);
  const spitfire::math::cColour lightPointColour(0.25f, 0.0f, 0.0f);
  const float lightPointAmbient = 0.15f;
  const float lightPointConstantAttenuation = 0.3f;
  const float lightPointLinearAttenuation = 0.007f;
  const float lightPointExpAttenuation = 0.00008f;

  // Blue spot light
  const spitfire::math::cVec3 lightSpotPosition(0.0f, -16.0f, 1.0f);
  const spitfire::math::cVec3 lightSpotDirection(0.0f, 1.0f, 0.0f);
  const spitfire::math::cColour lightSpotColour(0.0f, 0.0f, 0.25f);
  //const float lightSpotAmbient = 0.15f;
  //const float lightSpotConstantAttenuation = 0.3f;
  const float lightSpotLinearAttenuation = 0.002f;
  //const float lightSpotExpAttenuation = 0.00008f;
  const float fLightSpotLightConeAngle = 40.0f;
  const float lightSpotConeCosineAngle = cosf(spitfire::math::DegreesToRadians(fLightSpotLightConeAngle));

  // Material
  const spitfire::math::cColour materialAmbientColour(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour materialDiffuseColour(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour materialSpecularColour(1.0f, 1.0f, 1.0f);
  const float fMaterialShininess = 25.0f;
  
  // Use Cornflower blue as the fog colour
  // http://en.wikipedia.org/wiki/Cornflower_blue
  const spitfire::math::cColour fogColour(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f);
  const float fFogStart = 5.0f;
  const float fFogEnd = 15.0f;
  //const float fFogDensity = 0.5f;


  const uint32_t uiUpdateInputDelta = uint32_t(1000.0f / 120.0f);
  const uint32_t uiUpdateDelta = uint32_t(1000.0f / 60.0f);

  while (!bIsDone) {
    // Update state
    currentTime = spitfire::util::GetTimeMS();

    if ((currentTime - previousUpdateInputTime) > uiUpdateInputDelta) {
      // Update window events
      pWindow->ProcessEvents();

      if (pWindow->IsActive()) {
        // Hide the cursor
        pWindow->ShowCursor(false);

        // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
        pWindow->WarpCursorToMiddleOfScreen();
      } else {
        // Show the cursor
        pWindow->ShowCursor(true);
      }

      previousUpdateInputTime = currentTime;
    }

    if ((currentTime - previousUpdateTime) > uiUpdateDelta) {
      // Update the camera
      const float fDistance = 0.1f;
      if (bIsMovingForward) camera.MoveZ(fDistance);
      if (bIsMovingBackward) camera.MoveZ(-fDistance);
      if (bIsMovingLeft) camera.MoveX(-fDistance);
      if (bIsMovingRight) camera.MoveX(fDistance);

      // Update object rotation
      if (bIsRotating) fAngleRadians += float(uiUpdateDelta) * fRotationSpeed;

      rotation.SetFromAxisAngle(spitfire::math::v3Up, fAngleRadians);

      matObjectRotation.SetRotation(rotation);

      previousUpdateTime = currentTime;
    }

    if (bReloadShaders) {
      LOG("Reloading shaders");

      // Destroy and create our shaders
      DestroyShaders();
      CreateShaders();

      bReloadShaders = false;
    }

    if (bUpdateShaderConstants) {
      // Set our shader constants
      pContext->BindShader(*pShaderMetal);
        // Setup lighting
        pContext->SetShaderConstant("light.ambientColour", lightDirectionalAmbientColour);
        pContext->SetShaderConstant("light.diffuseColour", lightDirectionalDiffuseColour);
        pContext->SetShaderConstant("light.specularColour", lightDirectionalSpecularColour);

        // Setup materials
        pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
        pContext->SetShaderConstant("material.diffuseColour", materialDiffuseColour);
        pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
        pContext->SetShaderConstant("material.fShininess", fMaterialShininess);
      pContext->UnBindShader(*pShaderMetal);

      pContext->BindShader(*pShaderFog);
        pContext->SetShaderConstant("fog.colour", fogColour);
        pContext->SetShaderConstant("fog.fStart", fFogStart);
        pContext->SetShaderConstant("fog.fEnd", fFogEnd);
        //pContext->SetShaderConstant("fog.fDensity", fFogDensity);
      pContext->UnBindShader(*pShaderFog);

      pContext->BindShader(*pShaderLights);
        // Directional light
        pContext->SetShaderConstant("lightDirectional.ambientColour", lightDirectionalAmbientColour);
        pContext->SetShaderConstant("lightDirectional.diffuseColour", lightDirectionalDiffuseColour);
        pContext->SetShaderConstant("lightDirectional.specularColour", lightDirectionalSpecularColour);

        // Point light
        pContext->SetShaderConstant("lightPointLight.colour", lightPointColour);
        pContext->SetShaderConstant("lightPointLight.fAmbient", lightPointAmbient);
        pContext->SetShaderConstant("lightPointLight.fConstantAttenuation", lightPointConstantAttenuation);
        pContext->SetShaderConstant("lightPointLight.fLinearAttenuation", lightPointLinearAttenuation);
        pContext->SetShaderConstant("lightPointLight.fExpAttenuation", lightPointExpAttenuation);

        // Spot light
        pContext->SetShaderConstant("lightSpotLight.colour", lightSpotColour);
        //pContext->SetShaderConstant("lightSpotLight.fAmbient", lightSpotAmbient);
        //pContext->SetShaderConstant("lightSpotLight.fConstantAttenuation", lightSpotConstantAttenuation);
        pContext->SetShaderConstant("lightSpotLight.fLinearAttenuation", lightSpotLinearAttenuation);
        //pContext->SetShaderConstant("lightSpotLight.fExpAttenuation", lightSpotExpAttenuation);
        pContext->SetShaderConstant("lightSpotLight.fConeCosineAngle", lightSpotConeCosineAngle);

        // Setup materials
        pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
        pContext->SetShaderConstant("material.diffuseColour", materialDiffuseColour);
        pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
        pContext->SetShaderConstant("material.fShininess", fMaterialShininess);
      pContext->UnBindShader(*pShaderLights);

      pContext->BindShader(*parallaxNormalMap.pShader);
        // Directional light
        pContext->SetShaderConstant("directionalLight.ambientColour", lightDirectionalAmbientColour);
        pContext->SetShaderConstant("directionalLight.diffuseColour", lightDirectionalDiffuseColour);
        pContext->SetShaderConstant("directionalLight.specularColour", lightDirectionalSpecularColour);

        // Setup materials
        pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
        pContext->SetShaderConstant("material.diffuseColour", materialDiffuseColour);
        pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
        pContext->SetShaderConstant("material.fShininess", fMaterialShininess);
      pContext->UnBindShader(*parallaxNormalMap.pShader);

      bUpdateShaderConstants = false;
    }


    // Update our text
    CreateText();
    assert(textVBO.IsCompiled());

    const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

    const spitfire::math::cMat4 matView = camera.CalculateViewMatrix();

    const spitfire::math::cVec3 lightDirection = (spitfire::math::cVec3(0.0f, 0.0f, 0.0f) - lightDirectionalPosition).GetNormalised();

    // Set up the metal shader
    pContext->BindShader(*pShaderMetal);
      pContext->SetShaderConstant("matView", matView);
      pContext->SetShaderConstant("light.direction", lightDirection);
    pContext->UnBindShader(*pShaderMetal);

    // Set up the lights shader
    pContext->BindShader(*pShaderLights);
      pContext->SetShaderConstant("matView", matView);

      // Directional light
      pContext->SetShaderConstant("lightDirectional.bOn", bIsDirectionalLightOn ? 1 : 0);
      pContext->SetShaderConstant("lightDirectional.direction", matView * -lightDirection);

      // Point light
      pContext->SetShaderConstant("lightPointLight.bOn", bIsPointLightOn ? 1 : 0);
      pContext->SetShaderConstant("lightPointLight.position", lightPointPosition);

      // Spot light
      pContext->SetShaderConstant("lightSpotLight.bOn", bIsSpotLightOn ? 1 : 0);
      pContext->SetShaderConstant("lightSpotLight.position", matView * lightSpotPosition);
      pContext->SetShaderConstant("lightSpotLight.direction", matView * lightSpotDirection);
    pContext->UnBindShader(*pShaderLights);

    // Set up the pallax normal map shader
    pContext->BindShader(*parallaxNormalMap.pShader);
      pContext->SetShaderConstant("matView", matView);

      // Directional light
      pContext->SetShaderConstant("directionalLight.direction", lightDirection);
    pContext->UnBindShader(*parallaxNormalMap.pShader);

    // Set up the cube map shader
    pContext->BindShader(*pShaderCubeMap);
      //pContext->SetShaderConstant("matView", matView);
      pContext->SetShaderConstant("cameraPos", camera.GetPosition());
    pContext->UnBindShader(*pShaderCubeMap);

    if (bSimplePostRenderDirty) {
      // Destroy any existing shader
      if (pShaderScreenRectSimplePostRender != nullptr) {
        pContext->DestroyShader(pShaderScreenRectSimplePostRender);
        pShaderScreenRectSimplePostRender = nullptr;
      }

      if (GetActiveSimplePostRenderShadersCount() != 0) {
        std::map<std::string, int> mapDefinesToAdd;

        // Load the vertex shader
        std::string sVertexShaderText;
        spitfire::storage::ReadText(TEXT("shaders/passthrough2d.vert"), sVertexShaderText);

        // Create our fragment shader
        std::string sFragmentShaderText =
          "#version 330\n"
          "\n"
        ;

        // Add the colour blind define that says which colour blind mode we are using
        if (IsColourBlindSimplePostRenderShaderEnabled()) {
          const int iColourBlindMode = int(GetColourBlindModeDefineValue());
          mapDefinesToAdd["COLORBLIND_MODE"] = iColourBlindMode;
        }
        
        sFragmentShaderText +=
          "uniform sampler2DRect texUnit0; // Diffuse texture\n"
          "\n"
          "smooth in vec2 vertOutTexCoord;\n"
          "\n"
          "out vec4 fragmentColor;\n"
          "\n"
        ;

        const size_t n = simplePostRenderShaders.size();
        for (size_t i = 0; i < n; i++) {
          if (simplePostRenderShaders[i].bOn) {
            // Load the shader text
            std::string sTempFragmentShaderText;
            spitfire::storage::ReadText(simplePostRenderShaders[i].sFragmentShaderFilePath, sTempFragmentShaderText);

            // Skip the version
            if (spitfire::string::StartsWith(sTempFragmentShaderText, "#version")) {
              size_t index = 0;
              if (spitfire::string::Find(sTempFragmentShaderText, "\n", index)) sTempFragmentShaderText = sTempFragmentShaderText.substr(index);
            }

            // Rename the ApplyFilter function to ProcessFilter0..n
            // NOTE: We use ProcessFilter0..n instead of ApplyFilter0..n to avoid an endless loop in spitfire::string::Replace
            const std::string sFixedFragmentShaderText =
              "// " + spitfire::string::ToUTF8(simplePostRenderShaders[i].sName) +
              spitfire::string::Replace(sTempFragmentShaderText, "ApplyFilter", "ProcessFilter" + spitfire::string::ToUTF8(spitfire::string::ToString(i))) +
              "\n"
            ;

            // Add it to the existing fragment shader
            sFragmentShaderText += sFixedFragmentShaderText;
          }
        }


        // Add our main function that calls the ApplyShader0..n functions
        sFragmentShaderText +=
          "\n"
          "void main()\n"
          "{\n"
          "  vec4 colour = texture(texUnit0, vertOutTexCoord);\n"
          "\n"
        ;

        for (size_t i = 0; i < n; i++) {
          if (simplePostRenderShaders[i].bOn) sFragmentShaderText += "  colour = ProcessFilter" + spitfire::string::ToUTF8(spitfire::string::ToString(i)) + "(colour);\n";
        }

        sFragmentShaderText +=
          "\n"
          "  fragmentColor = colour;\n"
          "}\n"
          "\n"
        ;

        // Create the shader
        pShaderScreenRectSimplePostRender = pContext->CreateShaderFromText(sVertexShaderText, sFragmentShaderText, TEXT("shaders/"), mapDefinesToAdd);
      }

      bSimplePostRenderDirty = false;
    }

    {
      // Render a few items from the scene into the frame buffer object for use later
      const spitfire::math::cColour clearColour(0.0f, 1.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObjectTeapot);

      if (bIsWireframe) pContext->EnableWireframe();

      pContext->BindTextureCubeMap(0, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      pContext->SetShaderConstant("matModel", matTranslationCubeMappedTeapot);

      pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCubeMappedTeapot);

        pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
      }

      pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(0, *pTextureCubeMap);

      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToTexture(*pTextureFrameBufferObjectTeapot);
    }

    {
      // Render the scene into a texture for later
      // Use Cornflower blue as the background colour
      // http://en.wikipedia.org/wiki/Cornflower_blue
      const spitfire::math::cColour clearColour(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObjectScreen);

      if (bIsWireframe) pContext->EnableWireframe();


      // Render the cube mapped teapot
      pContext->BindTextureCubeMap(0, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      pContext->SetShaderConstant("matModel", matTranslationCubeMappedTeapot);

      pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCubeMappedTeapot);

        pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
      }

      pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(0, *pTextureCubeMap);


      {
        // Render the car paint teapot
        pContext->BindTexture(0, *pTextureDiffuse);
        pContext->BindTextureCubeMap(1, *pTextureCubeMap);

        pContext->BindShader(*pShaderCarPaint);

        // Set our constants
        const spitfire::math::cMat4 matModelView = matView * matTranslationCarPaintTeapot;
        pContext->SetShaderConstant("fvLightPosition", matModelView * lightDirectionalPosition);
        pContext->SetShaderConstant("fvEyePosition", spitfire::math::cVec3());// matModelView * camera.GetPosition());

        pContext->SetShaderConstant("cameraPos", camera.GetPosition());
        pContext->SetShaderConstant("matModel", matTranslationCarPaintTeapot);

        // The world matrix is the model matrix apparently, matObjectToBeRendered
        //const spitfire::math::cMat4 matWorld = matTranslationCarPaintTeapot;
        //const spitfire::math::cMat4 matWorldInverseTranspose = matWorld.GetInverseTranspose();
        //pContext->SetShaderConstant("matWorldInverseTranspose", matWorldInverseTranspose);

        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

        {
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCarPaintTeapot);

          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
        }

        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

        pContext->UnBindShader(*pShaderCarPaint);

        pContext->UnBindTextureCubeMap(1, *pTextureCubeMap);
        pContext->UnBindTexture(0, *pTextureDiffuse);
      }

      {
        // Render the cel shaded teapot which consists of a black silhouette pass and a cel shaded pass

        // Black silhouette
        {
          glEnable(GL_CULL_FACE); // enable culling
          glCullFace(GL_CW); // enable culling of front faces
          glDepthMask(GL_FALSE); // enable writes to Z-buffer

          pContext->BindShader(*pShaderSilhouette);

          // Set our constants
          const float fOffset = 0.25f;
          pContext->SetShaderConstant("fOffset", fOffset);

          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          {
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCelShadedTeapot);

            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
          }

          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          pContext->UnBindShader(*pShaderSilhouette);

          glCullFace(GL_CW); // enable culling of back faces
          glDepthMask(GL_FALSE); // disable writes to Z-buffer
          glEnable(GL_DEPTH_TEST);
          glDepthMask(GL_TRUE);
        }

        // Cel shaded
        {
          pContext->BindTexture(0, *pTextureCelShaderColourMap);

          pContext->BindShader(*pShaderCelShaded);

          // Set our constants
          const spitfire::math::cMat4 matModel = matTranslationCelShadedTeapot;
          const spitfire::math::cMat4 matViewProjection = matProjection * matView;
          pContext->SetShaderConstant("matModel", matModel);
          pContext->SetShaderConstant("matViewProjection", matViewProjection);

          pContext->SetShaderConstant("cameraPosition", camera.GetPosition());
          pContext->SetShaderConstant("lightPosition", lightPointPosition);
          pContext->SetShaderConstant("nShades", 6);
          pContext->SetShaderConstant("colour", spitfire::math::cVec3(1.0f, 0.62f, 0.0));

          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          {
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCelShadedTeapot);

            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
          }

          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          pContext->UnBindShader(*pShaderCelShaded);

          pContext->UnBindTexture(0, *pTextureCelShaderColourMap);
        }
      }


      #ifdef BUILD_LARGE_STATUE_MODEL
      // Render the statues
      {
        pContext->BindTexture(0, *pTextureMarble);

        pContext->BindShader(*pShaderLights);

        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectStatue);

        for (size_t i = 0; i < 9; i++) {
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationStatue[i] * matObjectRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectStatue);
        }

        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectStatue);

        pContext->UnBindShader(*pShaderLights);

        pContext->UnBindTexture(0, *pTextureMarble);
      }
      #endif


      // Render the parallax normal map cube
      {
        pContext->BindTexture(0, *pTextureNormalMapDiffuse);
        pContext->BindTexture(1, *pTextureNormalMapSpecular);
        pContext->BindTexture(2, *pTextureNormalMapNormal);
        pContext->BindTexture(3, *pTextureNormalMapHeight);

        pContext->BindShader(*parallaxNormalMap.pShader);

        pContext->BindStaticVertexBufferObject(parallaxNormalMap.vbo);

        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationParallaxNormalMap);

        pContext->DrawStaticVertexBufferObjectTriangles(parallaxNormalMap.vbo);

        pContext->UnBindStaticVertexBufferObject(parallaxNormalMap.vbo);

        pContext->UnBindShader(*parallaxNormalMap.pShader);

        pContext->UnBindTexture(3, *pTextureNormalMapHeight);
        pContext->UnBindTexture(2, *pTextureNormalMapNormal);
        pContext->UnBindTexture(1, *pTextureNormalMapSpecular);
        pContext->UnBindTexture(0, *pTextureNormalMapDiffuse);
      }


      // Render the lights
      {
        pContext->BindShader(*pShaderMetal);

        {
          pContext->SetShaderConstant("material.ambientColour", lightPointColour);

          spitfire::math::cMat4 matTransform;
          matTransform.SetTranslation(lightPointPosition);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPointLight);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTransform);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPointLight);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPointLight);
        }

        {
          pContext->SetShaderConstant("material.ambientColour", lightSpotColour);

          spitfire::math::cMat4 matTransform;
          matTransform.SetTranslation(lightSpotPosition);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSpotLight);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTransform);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSpotLight);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSpotLight);
        }

        pContext->UnBindShader(*pShaderMetal);
      }


      // Render the metal objects
      pContext->BindShader(*pShaderMetal);

      {
        pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[0] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane0);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[1] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube0);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[2] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox0);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[3] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[4] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
        }
      }

      pContext->UnBindShader(*pShaderMetal);


      // Render the textured objects
      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureLightMap);
      pContext->BindTexture(2, *pTextureDetail);

      pContext->BindShader(*pShaderCrate);

      {
        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[5] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[6] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[7] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[8] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[9] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
        }
      }

      pContext->UnBindShader(*pShaderCrate);

      pContext->UnBindTexture(2, *pTextureDetail);
      pContext->UnBindTexture(1, *pTextureLightMap);
      pContext->UnBindTexture(0, *pTextureDiffuse);


      // Render the foggy objects
      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);

      pContext->BindShader(*pShaderFog);

      {
        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[10] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[11] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[12] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[13] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[14] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
        }
      }

      pContext->UnBindShader(*pShaderFog);

      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);



      // Render the cubemapped objects




      // Render the cube mapped teapot
      pContext->BindTextureCubeMap(0, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      {
        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
          pContext->SetShaderConstant("matModel", matTranslationArray[15] * matObjectRotation);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[15] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube2);
          pContext->SetShaderConstant("matModel", matTranslationArray[16] * matObjectRotation);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[16] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox2);
          pContext->SetShaderConstant("matModel", matTranslationArray[17] * matObjectRotation);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[17] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
          pContext->SetShaderConstant("matModel", matTranslationArray[18] * matObjectRotation);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[18] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
          pContext->SetShaderConstant("matModel", matTranslationArray[19] * matObjectRotation);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[19] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
        }
      }

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(0, *pTextureCubeMap);



      // Render the lambert shaded objects
      pContext->BindShader(*pShaderLambert);

      {
        pContext->SetShaderConstant("lightPosition", lightDirectionalPosition);
        pContext->SetShaderConstant("matView", matView);

        // Mirror's Edge style lighting (But much uglier because this is just a cheap knock off)
        pContext->SetShaderConstant("ambientColour", spitfire::math::cColour3(0.0f, 0.35f, 1.0f));
        pContext->SetShaderConstant("lightColour", spitfire::math::cColour3(1.0f, 1.0f, 0.83f));
        
        const spitfire::math::cColour3 white(0.9f, 0.9f, 0.9f);
        const spitfire::math::cColour3 red(1.0f, 0.0f, 0.0f);

        {
          pContext->SetShaderConstant("colour", white);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[20] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane0);
        }

        {
          pContext->SetShaderConstant("colour", white);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[21] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube0);
        }

        {
          pContext->SetShaderConstant("colour", red);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[22] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox0);
        }

        {
          pContext->SetShaderConstant("colour", white);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[23] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
        }

        {
          pContext->SetShaderConstant("colour", red);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[24] * matObjectRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
        }
      }

      pContext->UnBindShader(*pShaderLambert);


      // Render the test images
      {
        const size_t n = testImages.size();
        for (size_t i = 0; i < n; i++) {
          cTextureVBOPair* pPair = testImages[i];

          pContext->BindTexture(0, *(pPair->pTexture));

          pContext->BindShader(*pShaderPassThrough);

          pContext->BindStaticVertexBufferObject(pPair->vbo);

          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationTestImages[i]);

          pContext->DrawStaticVertexBufferObjectTriangles(pPair->vbo);

          pContext->UnBindStaticVertexBufferObject(pPair->vbo);

          pContext->UnBindShader(*pShaderPassThrough);

          pContext->UnBindTexture(0, *(pPair->pTexture));
        }
      }

      // Render some gui in 3d space
      {
        pContext->DisableDepthTesting();

        const size_t n = testImages.size();
        for (size_t i = 0; i < n; i++) {
          cTextureVBOPair* pPair = testImages[i];

          pContext->BindTexture(0, *(pPair->pTexture));

          pContext->BindShader(*pShaderPassThrough);

          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);

          const spitfire::math::cVec3 position(2.5f - float(i), 1.8f, -5.0f);
          spitfire::math::cMat4 matTranslation;
          matTranslation.SetTranslation(position);

          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matTranslation);

          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectGuiRectangle);

          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);


          pContext->UnBindShader(*pShaderPassThrough);

          pContext->UnBindTexture(0, *(pPair->pTexture));
        }

        pContext->EnableDepthTesting();
      }

      pContext->EndRenderToTexture(*pTextureFrameBufferObjectScreen);
    }

    {
      // Render the frame buffer objects to the screen
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToScreen();

      // Now draw an overlay of our rendered textures
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);


      if (GetActiveSimplePostRenderShadersCount() != 0) {
        if (bIsSplitScreenSimplePostEffectShaders) {
          // Draw the screen texture
          RenderScreenRectangle(0.5f, 0.5f, staticVertexBufferObjectScreenRectScreen, *pTextureFrameBufferObjectScreen, *pShaderScreenRect);

          // Draw the shaders split screen texture
          RenderScreenRectangle(0.25f, 0.5f, staticVertexBufferObjectScreenRectHalfScreen, *pTextureFrameBufferObjectScreen, *pShaderScreenRectSimplePostRender);
        } else {
          // Draw the shaders screen texture
          RenderScreenRectangle(0.5f, 0.5f, staticVertexBufferObjectScreenRectScreen, *pTextureFrameBufferObjectScreen, *pShaderScreenRectSimplePostRender);
        }
      } else {
        // Draw the screen texture
        RenderScreenRectangle(0.5f, 0.5f, staticVertexBufferObjectScreenRectScreen, *pTextureFrameBufferObjectScreen, *pShaderScreenRect);
      }

      // Draw the teapot texture
      RenderScreenRectangle(0.75f + (0.5f * 0.25f), 0.75f + (0.5f * 0.25f), staticVertexBufferObjectScreenRectTeapot, *pTextureFrameBufferObjectTeapot, *pShaderScreenRect);


      // Draw the text overlay
      {
        pContext->BindFont(*pFont);

        // Rendering the font in the middle of the screen
        spitfire::math::cMat4 matModelView;
        matModelView.SetTranslation(0.02f, 0.05f, 0.0f);

        pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView);

        pContext->BindStaticVertexBufferObject2D(textVBO);

        {
          pContext->DrawStaticVertexBufferObjectTriangles2D(textVBO);
        }

        pContext->UnBindStaticVertexBufferObject2D(textVBO);

        pContext->UnBindFont(*pFont);
      }

      pContext->EndRenderMode2D();

      pContext->EndRenderToScreen(*pWindow);
    }

    // Gather our frames per second
    Frames++;
    {
      spitfire::durationms_t t = spitfire::util::GetTimeMS();
      if (t - T0 >= 5000) {
        #ifdef BUILD_DEBUG
        float seconds = (t - T0) / 1000.0f;
        float fps = Frames / seconds;
        LOG(Frames, " frames in ", seconds, " seconds = ", fps, " FPS");
        #endif
        T0 = t;
        Frames = 0;
      }
    }
  };

  pWindow->ShowCursor(true);
}

int main(int argc, char** argv)
{
  bool bIsSuccess = true;

  {
    cApplication application;

    bIsSuccess = application.Create();
    if (bIsSuccess) application.Run();

    application.Destroy();
  }

  return bIsSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}


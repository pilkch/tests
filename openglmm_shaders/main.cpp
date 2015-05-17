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

// TODO: GET A HEBE STATUE MODEL

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>

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
  spitfire::math::cVec3 GetPosition() const;
  void SetPosition(const spitfire::math::cVec3& position);
  void SetRotation(const spitfire::math::cQuaternion& rotation);

  void MoveX(float fDistance);
  void MoveY(float fDistance);
  void RotateX(float fDegrees);
  void RotateZ(float fDegrees);

  spitfire::math::cMat4 CalculateViewMatrix() const;

private:
  spitfire::math::cVec3 position;
  spitfire::math::cQuaternion rotation;
};

spitfire::math::cVec3 cFreeLookCamera::GetPosition() const
{
  return position;
}

void cFreeLookCamera::SetPosition(const spitfire::math::cVec3& _position)
{
  position = _position;
}

void cFreeLookCamera::SetRotation(const spitfire::math::cQuaternion& _rotation)
{
  rotation = _rotation;
}

void cFreeLookCamera::MoveX(float xmmod)
{
  position += rotation * spitfire::math::cVec3(xmmod, 0.0f, 0.0f);
}

void cFreeLookCamera::MoveY(float ymmod)
{
  position += rotation * spitfire::math::cVec3(0.0f, 0.0f, -ymmod);
}

void cFreeLookCamera::RotateX(float xrmod)
{
  spitfire::math::cQuaternion nrot;
  nrot.SetFromAxisAngleDegrees(spitfire::math::cVec3(1.0f, 0.0f, 0.0f), xrmod);
  rotation = rotation * nrot;
}

void cFreeLookCamera::RotateZ(float yrmod)
{
  spitfire::math::cQuaternion nrot;
  nrot.SetFromAxisAngleDegrees(spitfire::math::cVec3(0.0f, 0.0f, 1.0f), yrmod);
  rotation = nrot * rotation;
}

spitfire::math::cMat4 cFreeLookCamera::CalculateViewMatrix() const
{
  spitfire::math::cMat4 matTranslation;
  matTranslation.TranslateMatrix(-position);
  return (-rotation).GetMatrix() * matTranslation;
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


class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();

  bool Create();
  void Destroy();

  void Run();

private:
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

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  std::vector<std::string> GetInputDescription() const;

  bool bIsMovingForward;
  bool bIsMovingLeft;
  bool bIsMovingRight;
  bool bIsMovingBackward;

  bool bIsDirectionalLightOn;
  bool bIsPointLightOn;
  bool bIsSpotLightOn;

  bool bIsRotating;
  bool bIsWireframe;

  enum POSTEFFECT {
    NONE,
    FXAA,
    SEPIA,
    NOIR,
    MATRIX,
    TEAL_AND_ORANGE
  };
  POSTEFFECT postEffect;

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

  opengl::cTexture* pTextureNormalMapDiffuse;
  opengl::cTexture* pTextureNormalMapSpecular;
  opengl::cTexture* pTextureNormalMapNormal;
  opengl::cTexture* pTextureNormalMapHeight;

  opengl::cShader* pShaderCubeMap;
  opengl::cShader* pShaderLights;
  opengl::cShader* pShaderPassThrough;
  opengl::cShader* pShaderScreenRect;
  opengl::cShader* pShaderScreenRectFXAA;
  opengl::cShader* pShaderScreenRectSepia;
  opengl::cShader* pShaderScreenRectNoir;
  opengl::cShader* pShaderScreenRectMatrix;
  opengl::cShader* pShaderScreenRectTealAndOrange;

  opengl::cStaticVertexBufferObject staticVertexBufferObject;
  #ifdef BUILD_LARGE_STATUE_MODEL
  opengl::cStaticVertexBufferObject staticVertexBufferObjectStatue;
  #endif
  opengl::cStaticVertexBufferObject staticVertexBufferObjectScreenRectScreen;
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
};

cApplication::cApplication() :
  bIsMovingForward(false),
  bIsMovingLeft(false),
  bIsMovingRight(false),
  bIsMovingBackward(false),

  bIsDirectionalLightOn(true),
  bIsPointLightOn(true),
  bIsSpotLightOn(true),

  bIsRotating(true),
  bIsWireframe(false),

  postEffect(POSTEFFECT::NONE),

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

  pTextureNormalMapDiffuse(nullptr),
  pTextureNormalMapSpecular(nullptr),
  pTextureNormalMapNormal(nullptr),
  pTextureNormalMapHeight(nullptr),

  pShaderCubeMap(nullptr),
  pShaderLights(nullptr),
  pShaderPassThrough(nullptr),
  pShaderScreenRect(nullptr),
  pShaderScreenRectFXAA(nullptr),
  pShaderScreenRectSepia(nullptr),
  pShaderScreenRectNoir(nullptr),
  pShaderScreenRectMatrix(nullptr),
  pShaderScreenRectTealAndOrange(nullptr),

  pShaderCrate(nullptr),
  pShaderFog(nullptr),
  pShaderMetal(nullptr)
{
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

  spitfire::string_t sPostRenderEffect = TEXT("None");
  switch (postEffect) {
    case FXAA:
      sPostRenderEffect = TEXT("FXAA");
      break;
    case SEPIA:
      sPostRenderEffect = TEXT("Sepia");
      break;
    case NOIR:
      sPostRenderEffect = TEXT("Noir");
      break;
    case MATRIX:
      sPostRenderEffect = TEXT("Matrix");
      break;
    case TEAL_AND_ORANGE:
      sPostRenderEffect = TEXT("Teal and Orange");
      break;
  };

  lines.push_back(TEXT("Post Render Effect: ") + sPostRenderEffect);


  // Add our lines of text
  const spitfire::math::cColour blue(0.0f, 0.0f, 1.0f);
  float y = 0.0f;
  std::list<spitfire::string_t>::const_iterator iter(lines.begin());
  const std::list<spitfire::string_t>::const_iterator iterEnd(lines.end());
  while (iter != iterEnd) {
    pFont->PushBack(builder, *iter, blue, spitfire::math::cVec2(0.0f, y));
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
  builder.CreateTeapot(fRadius, nSegments, *pGeometryDataPtr, 2);

  staticVertexBufferObject.SetData(pGeometryDataPtr);

  staticVertexBufferObject.Compile();
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

  // The obj file uses Y up, so we rotate all the points and normals within it
  spitfire::math::cQuaternion rotation;
  rotation.SetFromAxisAngleDegrees(spitfire::math::v3Left, 90.0f);

  spitfire::math::cMat4 matRotation;
  matRotation.SetRotation(rotation);

  // The obj file is giant so scale it down as well
  spitfire::math::cMat4 matScale;
  matScale.SetScale(spitfire::math::cVec3(0.1f, 0.1f, 0.1f));

  const spitfire::math::cMat4 matTransform = matRotation * matScale;

  const size_t nMeshes = model.mesh.size();
  for (size_t iMesh = 0; iMesh < nMeshes; iMesh++) {
    const size_t nVertices = model.mesh[iMesh]->vertices.size() / 3;
    for (size_t iVertex = 0; iVertex < nVertices; iVertex++) {
      builder.PushBack(
        matTransform * spitfire::math::cVec3(model.mesh[iMesh]->vertices[(3 * iVertex)], model.mesh[iMesh]->vertices[(3 * iVertex) + 1], model.mesh[iMesh]->vertices[(3 * iVertex) + 2]),
        matRotation * spitfire::math::cVec3(model.mesh[iMesh]->normals[(3 * iVertex)], model.mesh[iMesh]->normals[(3 * iVertex) + 1], model.mesh[iMesh]->normals[(3 * iVertex) + 2]),
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
  const spitfire::math::cVec3 vMin(-fHalfWidth, 0.0f, -fHalfHeight);
  const spitfire::math::cVec3 vMax(fHalfWidth, 0.0f, fHalfHeight);
  const spitfire::math::cVec3 vNormal(0.0f, -1.0f, 0.0f);

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec3(vMax.x, 0.0f, vMin.z), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMin.x, 0.0f, vMax.z), vNormal, spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMax.x, 0.0f, vMax.z), vNormal, spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, 0.0f, vMin.z), vNormal, spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMin.x, 0.0f, vMax.z), vNormal, spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMax.x, 0.0f, vMin.z), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));

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

  // Override the resolution
  opengl::cSystem::GetWindowedTestResolution16By9(resolution.width, resolution.height);
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
    TEXT("textures/cubemap_positive_x.png"),
    TEXT("textures/cubemap_negative_x.png"),
    TEXT("textures/cubemap_positive_y.png"),
    TEXT("textures/cubemap_negative_y.png"),
    TEXT("textures/cubemap_positive_z.png"),
    TEXT("textures/cubemap_negative_z.png")
  );
  assert(pTextureCubeMap != nullptr);

  pTextureMarble = pContext->CreateTexture(TEXT("textures/marble.png"));
  assert(pTextureMarble != nullptr);

  pTextureNormalMapDiffuse = pContext->CreateTexture(TEXT("textures/floor_tile_color_map.png"));
  assert(pTextureNormalMapDiffuse != nullptr);

  pTextureNormalMapSpecular = pContext->CreateTexture(TEXT("textures/floor_tile_gloss_map.png"));
  assert(pTextureNormalMapSpecular != nullptr);

  pTextureNormalMapNormal = pContext->CreateTexture(TEXT("textures/floor_tile_normal_map.png"));
  assert(pTextureNormalMapNormal != nullptr);

  pTextureNormalMapHeight = pContext->CreateTexture(TEXT("textures/floor_tile_height_map.png"));
  assert(pTextureNormalMapHeight != nullptr);

  pShaderCubeMap = pContext->CreateShader(TEXT("shaders/cubemap.vert"), TEXT("shaders/cubemap.frag"));
  assert(pShaderCubeMap != nullptr);

  pShaderLights = pContext->CreateShader(TEXT("shaders/lights.vert"), TEXT("shaders/lights.frag"));
  assert(pShaderLights != nullptr);

  parallaxNormalMap.pShader = pContext->CreateShader(TEXT("shaders/parallaxnormalmap.vert"), TEXT("shaders/parallaxnormalmap.frag"));
  assert(parallaxNormalMap.pShader != nullptr);

  pShaderPassThrough = pContext->CreateShader(TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(pShaderPassThrough != nullptr);

  pShaderScreenRect = pContext->CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/passthrough2d.frag"));
  assert(pShaderScreenRect != nullptr);

  pShaderScreenRectFXAA = pContext->CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/fxaa.frag"));
  assert(pShaderScreenRectFXAA != nullptr);

  pShaderScreenRectSepia = pContext->CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/sepia.frag"));
  assert(pShaderScreenRectSepia != nullptr);

  pShaderScreenRectNoir = pContext->CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/noir.frag"));
  assert(pShaderScreenRectNoir != nullptr);

  pShaderScreenRectMatrix = pContext->CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/matrix.frag"));
  assert(pShaderScreenRectMatrix != nullptr);

  pShaderScreenRectTealAndOrange = pContext->CreateShader(TEXT("shaders/passthrough2d.vert"), TEXT("shaders/tealandorange.frag"));
  assert(pShaderScreenRectTealAndOrange != nullptr);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObject);
  CreateTeapotVBO();

  #ifdef BUILD_LARGE_STATUE_MODEL
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectStatue);
  CreateStatueVBO();
  #endif

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectScreen);
  CreateScreenRectVBO(staticVertexBufferObjectScreenRectScreen, 1.0f, 1.0f);

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


  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  pShaderFog = pContext->CreateShader(TEXT("shaders/fog.vert"), TEXT("shaders/fog.frag"));
  pShaderMetal = pContext->CreateShader(TEXT("shaders/metal.vert"), TEXT("shaders/metal.frag"));

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


  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectTeapot);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectScreen);

  #ifdef BUILD_LARGE_STATUE_MODEL
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectStatue);
  #endif

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObject);


  if (pShaderScreenRectTealAndOrange != nullptr) {
    pContext->DestroyShader(pShaderScreenRectTealAndOrange);
    pShaderScreenRectTealAndOrange = nullptr;
  }
  if (pShaderScreenRectMatrix != nullptr) {
    pContext->DestroyShader(pShaderScreenRectMatrix);
    pShaderScreenRectMatrix = nullptr;
  }
  if (pShaderScreenRectNoir != nullptr) {
    pContext->DestroyShader(pShaderScreenRectNoir);
    pShaderScreenRectNoir = nullptr;
  }
  if (pShaderScreenRectFXAA != nullptr) {
    pContext->DestroyShader(pShaderScreenRectFXAA);
    pShaderScreenRectFXAA = nullptr;
  }
  if (pShaderScreenRectSepia != nullptr) {
    pContext->DestroyShader(pShaderScreenRectSepia);
    pShaderScreenRectSepia = nullptr;
  }
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

  if (pShaderLights != nullptr) {
    pContext->DestroyShader(pShaderLights);
    pShaderLights = nullptr;
  }

  if (pShaderCubeMap != nullptr) {
    pContext->DestroyShader(pShaderCubeMap);
    pShaderCubeMap = nullptr;
  }

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

  pContext = nullptr;

  if (pWindow != nullptr) {
    system.DestroyWindow(pWindow);
    pWindow = nullptr;
  }
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
      camera.RotateZ(-0.08f * (event.GetX() - (pWindow->GetWidth() * 0.5f)));
    }

    if (fabs(event.GetY() - (pWindow->GetHeight() * 0.5f)) > 1.5f) {
      camera.RotateX(-0.05f * (event.GetY() - (pWindow->GetHeight() * 0.5f)));
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
      case SDLK_y: {
        // Cycle backwards through the post render effects
        if (postEffect == POSTEFFECT::NONE) postEffect = POSTEFFECT::FXAA;
        else if (postEffect == POSTEFFECT::FXAA) postEffect = POSTEFFECT::SEPIA;
        else if (postEffect == POSTEFFECT::SEPIA) postEffect = POSTEFFECT::NOIR;
        else if (postEffect == POSTEFFECT::NOIR) postEffect = POSTEFFECT::MATRIX;
        else if (postEffect == POSTEFFECT::MATRIX) postEffect = POSTEFFECT::TEAL_AND_ORANGE;
        else postEffect = POSTEFFECT::NONE;
        break;
      }
      case SDLK_t: {
        // Cycle backwards through the post render effects
        if (postEffect == POSTEFFECT::TEAL_AND_ORANGE) postEffect = POSTEFFECT::MATRIX;
        else if (postEffect == POSTEFFECT::MATRIX) postEffect = POSTEFFECT::NOIR;
        else if (postEffect == POSTEFFECT::NOIR) postEffect = POSTEFFECT::SEPIA;
        else if (postEffect == POSTEFFECT::SEPIA) postEffect = POSTEFFECT::FXAA;
        else if (postEffect == POSTEFFECT::FXAA) postEffect = POSTEFFECT::NONE;
        else postEffect = POSTEFFECT::TEAL_AND_ORANGE;
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
  description.push_back("1 toggle wireframe");
  description.push_back("2 toggle directional light");
  description.push_back("3 toggle point light");
  description.push_back("4 toggle spot light");
  description.push_back("Y/U switch shader (None, FXAA, sepia, noir, matrix, teal and orange)");
  description.push_back("Esc quit");

  return description;
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
  assert(pShaderLights != nullptr);
  assert(pShaderLights->IsCompiledProgram());
  assert(parallaxNormalMap.pShader != nullptr);
  assert(parallaxNormalMap.pShader->IsCompiledProgram());
  assert(pShaderPassThrough != nullptr);
  assert(pShaderPassThrough->IsCompiledProgram());
  assert(pShaderScreenRect != nullptr);
  assert(pShaderScreenRect->IsCompiledProgram());
  assert(pShaderScreenRectFXAA != nullptr);
  assert(pShaderScreenRectFXAA->IsCompiledProgram());
  assert(pShaderScreenRectSepia != nullptr);
  assert(pShaderScreenRectSepia->IsCompiledProgram());
  assert(pShaderScreenRectNoir != nullptr);
  assert(pShaderScreenRectNoir->IsCompiledProgram());
  assert(pShaderScreenRectMatrix != nullptr);
  assert(pShaderScreenRectMatrix->IsCompiledProgram());
  assert(pShaderScreenRectTealAndOrange != nullptr);
  assert(pShaderScreenRectTealAndOrange->IsCompiledProgram());

  assert(staticVertexBufferObject.IsCompiled());
  #ifdef BUILD_LARGE_STATUE_MODEL
  assert(staticVertexBufferObjectStatue.IsCompiled());
  #endif
  assert(staticVertexBufferObjectScreenRectScreen.IsCompiled());
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
  spitfire::math::cQuaternion cameraRotationZ;
  cameraRotationZ.SetFromAxisAngleDegrees(spitfire::math::v3Up, -90.0f);
  spitfire::math::cQuaternion cameraRotationX;
  cameraRotationX.SetFromAxisAngleDegrees(spitfire::math::v3Left, 45.0f);
  camera.SetRotation(cameraRotationZ * cameraRotationX);

  // Set up the translations for our objects
  const size_t columns = 5;
  const size_t rows = 3;

  const float fSpacingX = 0.007f * pContext->GetWidth() / float(rows);
  const float fSpacingY = 0.03f * pContext->GetHeight() / float(columns);
  const float fLeft = -0.5f * float(columns - 1) * fSpacingX;

  spitfire::math::cVec3 positions[columns * rows];
  size_t i = 0;
  for (size_t y = 0; y < rows; y++) {
    for (size_t x = 0; x < columns; x++) {
      positions[i].Set(fLeft + (x * fSpacingX), (y * fSpacingY), 0.0f);
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
  const spitfire::math::cVec3 positionCubeMappedTeapot(-fSpacingX, (-1.0f * fSpacingY), 0.0f);
  spitfire::math::cMat4 matTranslationCubeMappedTeapot;
  matTranslationCubeMappedTeapot.SetTranslation(positionCubeMappedTeapot);

  // Parallax normal mapping
  const spitfire::math::cVec3 parallaxNormalMapPosition(fSpacingX, (-1.0f * fSpacingY), 0.0f);
  spitfire::math::cMat4 matTranslationParallaxNormalMap;
  matTranslationParallaxNormalMap.SetTranslation(parallaxNormalMapPosition);

  #ifdef BUILD_LARGE_STATUE_MODEL
  spitfire::math::cMat4 matTranslationStatue[9];
  i = 0;
  for (size_t y = 0; y < 3; y++) {
    for (size_t x = 0; x < 3; x++) {
      const spitfire::math::cVec3 position((-1.0f + float(x)) * fSpacingX, ((-2.0f - float(y)) * fSpacingY), 0.0f);
      matTranslationStatue[i].SetTranslation(position);
      i++;
    }
  }
  #endif

  std::vector<spitfire::math::cMat4> matTranslationTestImages;
  {
    const size_t n = testImages.size();
    for (size_t i = 0; i < n; i++) {
      const spitfire::math::cVec3 position((-1.0f + float(i)) * fSpacingX, -5.0f * fSpacingY, 0.0f);
      spitfire::math::cMat4 matTranslation;
      matTranslation.SetTranslation(position);
      matTranslationTestImages.push_back(matTranslation);
    }
  }


  uint32_t T0 = 0;
  uint32_t Frames = 0;

  uint32_t previousUpdateInputTime = SDL_GetTicks();
  uint32_t previousUpdateTime = SDL_GetTicks();
  uint32_t currentTime = SDL_GetTicks();


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

  const spitfire::math::cColour fogColour(1.0f, 0.0f, 0.0f);
  const float fFogStart = 5.0f;
  const float fFogEnd = 20.0f;
  //const float fFogDensity = 0.5f;

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


  const uint32_t uiUpdateInputDelta = uint32_t(1000.0f / 120.0f);
  const uint32_t uiUpdateDelta = uint32_t(1000.0f / 60.0f);

  // Setup mouse
  pWindow->ShowCursor(false);
  pWindow->WarpCursorToMiddleOfScreen();

  while (!bIsDone) {
    // Update state
    currentTime = SDL_GetTicks();

    if ((currentTime - previousUpdateInputTime) > uiUpdateInputDelta) {
      // Update window events
      pWindow->ProcessEvents();

      // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
      pWindow->WarpCursorToMiddleOfScreen();

      previousUpdateInputTime = currentTime;
    }

    if ((currentTime - previousUpdateTime) > uiUpdateDelta) {
      // Update the camera
      const float fDistance = 0.1f;
      if (bIsMovingForward) camera.MoveY(fDistance);
      if (bIsMovingBackward) camera.MoveY(-fDistance);
      if (bIsMovingLeft) camera.MoveX(-fDistance);
      if (bIsMovingRight) camera.MoveX(fDistance);

      // Update object rotation
      if (bIsRotating) fAngleRadians += float(uiUpdateDelta) * fRotationSpeed;

      rotation.SetFromAxisAngle(spitfire::math::v3Up, fAngleRadians);

      matObjectRotation.SetRotation(rotation);

      previousUpdateTime = currentTime;
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

    // Set up the cube map shader
    pContext->BindShader(*pShaderCubeMap);
      pContext->SetShaderConstant("cameraPosition", matView * spitfire::math::cVec3(0.0f, 0.0f, 0.0f));
    pContext->UnBindShader(*pShaderCubeMap);

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

    {
      // Render a few items from the scene into the frame buffer object for use later
      const spitfire::math::cColour clearColour(0.0f, 1.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObjectTeapot);

      if (bIsWireframe) pContext->EnableWireframe();

      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);
      pContext->BindTextureCubeMap(2, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      pContext->BindStaticVertexBufferObject(staticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCubeMappedTeapot);

        pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(staticVertexBufferObject);

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(2, *pTextureCubeMap);
      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);

      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToTexture(*pTextureFrameBufferObjectTeapot);
    }

    {
      // Render the scene into a texture for later
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObjectScreen);

      if (bIsWireframe) pContext->EnableWireframe();


      // Render the teapot
      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);
      pContext->BindTextureCubeMap(2, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      pContext->BindStaticVertexBufferObject(staticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCubeMappedTeapot);

        pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(staticVertexBufferObject);

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(2, *pTextureCubeMap);
      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);

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


      // Render the cubemapped objects
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


      pContext->EndRenderToTexture(*pTextureFrameBufferObjectScreen);
    }

    {
      // Render the frame buffer objects to the screen
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToScreen();

      // Now draw an overlay of our rendered textures
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      // Draw the screen texture
      {
        opengl::cShader* pShader = pShaderScreenRect;

        if (postEffect == POSTEFFECT::FXAA) pShader = pShaderScreenRectFXAA;
        else if (postEffect == POSTEFFECT::SEPIA) pShader = pShaderScreenRectSepia;
        else if (postEffect == POSTEFFECT::NOIR) pShader = pShaderScreenRectNoir;
        else if (postEffect == POSTEFFECT::MATRIX) pShader = pShaderScreenRectMatrix;
        else if (postEffect == POSTEFFECT::TEAL_AND_ORANGE) pShader = pShaderScreenRectTealAndOrange;

        spitfire::math::cMat4 matModelView2D;
        matModelView2D.SetTranslation(0.5f, 0.5f, 0.0f);

        pContext->BindTexture(0, *pTextureFrameBufferObjectScreen);

        pContext->BindShader(*pShader);

        pContext->BindStaticVertexBufferObject2D(staticVertexBufferObjectScreenRectScreen);

        {
          pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

          pContext->DrawStaticVertexBufferObjectTriangles2D(staticVertexBufferObjectScreenRectScreen);
        }

        pContext->UnBindStaticVertexBufferObject2D(staticVertexBufferObjectScreenRectScreen);

        pContext->UnBindShader(*pShader);

        pContext->UnBindTexture(0, *pTextureFrameBufferObjectScreen);
      }

      // Draw the teapot texture
      {
        // Move the rectangle into the bottom right hand corner of the screen
        spitfire::math::cMat4 matModelView2D;
        matModelView2D.SetTranslation(0.75f + (0.5f * 0.25f), 0.75f + (0.5f * 0.25f), 0.0f);

        pContext->BindTexture(0, *pTextureFrameBufferObjectTeapot);

        pContext->BindShader(*pShaderScreenRect);

        pContext->BindStaticVertexBufferObject2D(staticVertexBufferObjectScreenRectTeapot);

        {
          pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

          pContext->DrawStaticVertexBufferObjectTriangles2D(staticVertexBufferObjectScreenRectTeapot);
        }

        pContext->UnBindStaticVertexBufferObject2D(staticVertexBufferObjectScreenRectTeapot);

        pContext->UnBindShader(*pShaderScreenRect);

        pContext->UnBindTexture(0, *pTextureFrameBufferObjectTeapot);
      }

      
      // Draw the text overlay
      {
        pContext->BindFont(*pFont);

        // Rendering the font in the middle of the screen
        spitfire::math::cMat4 matModelView;
        matModelView.SetTranslation(0.02f, 0.02f, 0.0f);

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
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 5000) {
        float seconds = (t - T0) / 1000.0f;
        float fps = Frames / seconds;
        LOG(Frames, " frames in ", seconds, " seconds = ", fps, " FPS");
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


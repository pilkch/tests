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
#include <SDL/SDL_image.h>

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

// Breathe headers
#include <breathe/render/model/cFileFormatOBJ.h>
#include <breathe/render/model/cStatic.h>

// libopenglmm headers
#include <libopenglmm/libopenglmm.h>
#include <libopenglmm/cContext.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

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

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();
  ~cApplication();

  bool Create();
  void Destroy();

  void Run();

private:
  void CreatePlane(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates);
  void CreateCube(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates);
  void CreateBox(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates);
  void CreateSphere(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates, float fRadius);
  void CreateTeapot(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates);
  void CreateGear(opengl::cStaticVertexBufferObject* pObject);

  void CreateNormalMappedCube();

  void CreateTeapotVBO();
  void CreateStatueVBO();
  void CreateScreenRectVBO();

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
  bool bIsDone;

  opengl::cSystem system;

  opengl::cResolution resolution;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  cFreeLookCamera camera;

  opengl::cTextureFrameBufferObject* pTextureFrameBufferObject;

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
  opengl::cShader* pShaderParallaxNormalMap;
  opengl::cShader* pShaderScreenRect;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObject;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectStatue;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectScreenRect;


  opengl::cShader* pShaderCrate;
  opengl::cShader* pShaderFog;
  opengl::cShader* pShaderMetal;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPlane0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectCube0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBox0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectSphere0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectTeapot0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectGear0;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPlane2;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectCube2;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBox2;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectSphere2;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectTeapot2;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPlane3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectCube3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBox3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectSphere3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectTeapot3;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPointLight;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectSpotLight;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectParallaxNormalMap;
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
  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  pTextureFrameBufferObject(nullptr),

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
  pShaderParallaxNormalMap(nullptr),
  pShaderScreenRect(nullptr),

  pStaticVertexBufferObject(nullptr),
  pStaticVertexBufferObjectStatue(nullptr),
  pStaticVertexBufferObjectScreenRect(nullptr),


  pShaderCrate(nullptr),
  pShaderFog(nullptr),
  pShaderMetal(nullptr),

  pStaticVertexBufferObjectPlane0(nullptr),
  pStaticVertexBufferObjectCube0(nullptr),
  pStaticVertexBufferObjectBox0(nullptr),
  pStaticVertexBufferObjectSphere0(nullptr),
  pStaticVertexBufferObjectTeapot0(nullptr),
  pStaticVertexBufferObjectGear0(nullptr),

  pStaticVertexBufferObjectPlane2(nullptr),
  pStaticVertexBufferObjectCube2(nullptr),
  pStaticVertexBufferObjectBox2(nullptr),
  pStaticVertexBufferObjectSphere2(nullptr),
  pStaticVertexBufferObjectTeapot2(nullptr),

  pStaticVertexBufferObjectPlane3(nullptr),
  pStaticVertexBufferObjectCube3(nullptr),
  pStaticVertexBufferObjectBox3(nullptr),
  pStaticVertexBufferObjectSphere3(nullptr),
  pStaticVertexBufferObjectTeapot3(nullptr),

  pStaticVertexBufferObjectPointLight(nullptr),
  pStaticVertexBufferObjectSpotLight(nullptr),
  pStaticVertexBufferObjectParallaxNormalMap(nullptr)
{
}

cApplication::~cApplication()
{
  Destroy();
}

void cApplication::CreatePlane(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreatePlane(fWidth, fDepth, *pGeometryDataPtr, nTextureCoordinates);

  pObject->SetData(pGeometryDataPtr);

  pObject->Compile(system);
}

void cApplication::CreateCube(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateCube(fWidth, *pGeometryDataPtr, nTextureCoordinates);

  pObject->SetData(pGeometryDataPtr);

  pObject->Compile(system);
}

void cApplication::CreateBox(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;
  const float fHeight = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateBox(fWidth, fDepth, fHeight, *pGeometryDataPtr, nTextureCoordinates);

  pObject->SetData(pGeometryDataPtr);

  pObject->Compile(system);
}

void cApplication::CreateSphere(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates, float fRadius)
{
  assert(pObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const size_t nSegments = 30;

  opengl::cGeometryBuilder builder;
  builder.CreateSphere(fRadius, nSegments, *pGeometryDataPtr, nTextureCoordinates);

  pObject->SetData(pGeometryDataPtr);

  pObject->Compile(system);
}

void cApplication::CreateTeapot(opengl::cStaticVertexBufferObject* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fRadius = 1.0f;
  const size_t nSegments = 20;

  opengl::cGeometryBuilder builder;
  builder.CreateTeapot(fRadius, nSegments, *pGeometryDataPtr, nTextureCoordinates);

  pObject->SetData(pGeometryDataPtr);

  pObject->Compile(system);
}

void cApplication::CreateGear(opengl::cStaticVertexBufferObject* pObject)
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
  assert(pStaticVertexBufferObjectParallaxNormalMap != nullptr);

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

  pStaticVertexBufferObjectParallaxNormalMap->SetData(pGeometryDataPtr);

  pStaticVertexBufferObjectParallaxNormalMap->Compile(system);
}

void cApplication::CreateTeapotVBO()
{
  assert(pStaticVertexBufferObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder builder;

  const float fRadius = 0.3f;
  const size_t nSegments = 20;
  builder.CreateTeapot(fRadius, nSegments, *pGeometryDataPtr, 2);

  pStaticVertexBufferObject->SetData(pGeometryDataPtr);

  pStaticVertexBufferObject->Compile(system);
}

void cApplication::CreateStatueVBO()
{
  assert(pStaticVertexBufferObjectStatue != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  breathe::render::model::cStaticModel model;

  breathe::render::model::cFileFormatOBJ loader;
  if (!loader.Load(TEXT("models/venus.obj"), model)) {
    std::cerr<<"cApplication::CreateStatueVBO Failed to load obj file"<<std::endl;
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

  pStaticVertexBufferObjectStatue->SetData(pGeometryDataPtr);

  pStaticVertexBufferObjectStatue->Compile(system);
}

void cApplication::CreateScreenRectVBO()
{
  assert(pStaticVertexBufferObjectScreenRect != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fTextureWidth = float(resolution.width);
  const float fTextureHeight = float(resolution.height);

  const float_t fWidth = 0.250f;
  const float_t fHeight = 0.250f;
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

  pStaticVertexBufferObjectScreenRect->SetData(pGeometryDataPtr);

  pStaticVertexBufferObjectScreenRect->Compile2D(system);
}

bool cApplication::Create()
{
  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 720) || (resolution.height < 480) || (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8)) {
    std::cout<<"Current screen resolution is not adequate "<<resolution.width<<"x"<<resolution.height<<std::endl;
    return false;
  }

  // Override the resolution
  opengl::cSystem::GetWindowedTestResolution16By9(resolution.width, resolution.height);
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  // Set our required resolution
  pWindow = system.CreateWindow(TEXT("OpenGLmm Shaders Test"), resolution, false);
  if (pWindow == nullptr) {
    std::cout<<"Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }

  pTextureFrameBufferObject = pContext->CreateTextureFrameBufferObject(resolution.width, resolution.height, opengl::PIXELFORMAT::R8G8B8A8);
  assert(pTextureFrameBufferObject != nullptr);

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

  pShaderParallaxNormalMap = pContext->CreateShader(TEXT("shaders/parallaxnormalmap.vert"), TEXT("shaders/parallaxnormalmap.frag"));
  assert(pShaderParallaxNormalMap != nullptr);

  pShaderScreenRect = pContext->CreateShader(TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(pShaderScreenRect != nullptr);

  pStaticVertexBufferObject = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObject != nullptr);
  CreateTeapotVBO();

  pStaticVertexBufferObjectStatue = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectStatue != nullptr);
  CreateStatueVBO();

  pStaticVertexBufferObjectScreenRect = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectScreenRect != nullptr);
  CreateScreenRectVBO();


  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  pShaderFog = pContext->CreateShader(TEXT("shaders/fog.vert"), TEXT("shaders/fog.frag"));
  pShaderMetal = pContext->CreateShader(TEXT("shaders/metal.vert"), TEXT("shaders/metal.frag"));

  const float fRadius = 1.0f;

  pStaticVertexBufferObjectPlane0 = pContext->CreateStaticVertexBufferObject();
  CreatePlane(pStaticVertexBufferObjectPlane0, 0);
  pStaticVertexBufferObjectCube0 = pContext->CreateStaticVertexBufferObject();
  CreateCube(pStaticVertexBufferObjectCube0, 0);
  pStaticVertexBufferObjectBox0 = pContext->CreateStaticVertexBufferObject();
  CreateBox(pStaticVertexBufferObjectBox0, 0);
  pStaticVertexBufferObjectSphere0 = pContext->CreateStaticVertexBufferObject();
  CreateSphere(pStaticVertexBufferObjectSphere0, 0, fRadius);
  pStaticVertexBufferObjectTeapot0 = pContext->CreateStaticVertexBufferObject();
  CreateTeapot(pStaticVertexBufferObjectTeapot0, 0);
  pStaticVertexBufferObjectGear0 = pContext->CreateStaticVertexBufferObject();
  CreateGear(pStaticVertexBufferObjectGear0);

  pStaticVertexBufferObjectPlane2 = pContext->CreateStaticVertexBufferObject();
  CreatePlane(pStaticVertexBufferObjectPlane2, 2);
  pStaticVertexBufferObjectCube2 = pContext->CreateStaticVertexBufferObject();
  CreateCube(pStaticVertexBufferObjectCube2, 2);
  pStaticVertexBufferObjectBox2 = pContext->CreateStaticVertexBufferObject();
  CreateBox(pStaticVertexBufferObjectBox2, 2);
  pStaticVertexBufferObjectSphere2 = pContext->CreateStaticVertexBufferObject();
  CreateSphere(pStaticVertexBufferObjectSphere2, 2, fRadius);
  pStaticVertexBufferObjectTeapot2 = pContext->CreateStaticVertexBufferObject();
  CreateTeapot(pStaticVertexBufferObjectTeapot2, 2);

  pStaticVertexBufferObjectPlane3 = pContext->CreateStaticVertexBufferObject();
  CreatePlane(pStaticVertexBufferObjectPlane3, 3);
  pStaticVertexBufferObjectCube3 = pContext->CreateStaticVertexBufferObject();
  CreateCube(pStaticVertexBufferObjectCube3, 3);
  pStaticVertexBufferObjectBox3 = pContext->CreateStaticVertexBufferObject();
  CreateBox(pStaticVertexBufferObjectBox3, 3);
  pStaticVertexBufferObjectSphere3 = pContext->CreateStaticVertexBufferObject();
  CreateSphere(pStaticVertexBufferObjectSphere3, 3, fRadius);
  pStaticVertexBufferObjectTeapot3 = pContext->CreateStaticVertexBufferObject();
  CreateTeapot(pStaticVertexBufferObjectTeapot3, 3);

  pStaticVertexBufferObjectPointLight = pContext->CreateStaticVertexBufferObject();
  CreateSphere(pStaticVertexBufferObjectPointLight, 0, 0.3f);

  pStaticVertexBufferObjectSpotLight = pContext->CreateStaticVertexBufferObject();
  CreateSphere(pStaticVertexBufferObjectSpotLight, 0, 0.3f);

  pStaticVertexBufferObjectParallaxNormalMap = pContext->CreateStaticVertexBufferObject();
  CreateNormalMappedCube();

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  if (pStaticVertexBufferObjectParallaxNormalMap != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectParallaxNormalMap);
    pStaticVertexBufferObjectParallaxNormalMap = nullptr;
  }

  if (pStaticVertexBufferObjectSpotLight != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectSpotLight);
    pStaticVertexBufferObjectSpotLight = nullptr;
  }
  if (pStaticVertexBufferObjectPointLight != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectPointLight);
    pStaticVertexBufferObjectPointLight = nullptr;
  }

  if (pStaticVertexBufferObjectTeapot3 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectTeapot3);
    pStaticVertexBufferObjectTeapot3 = nullptr;
  }
  if (pStaticVertexBufferObjectSphere3 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectSphere3);
    pStaticVertexBufferObjectSphere3 = nullptr;
  }
  if (pStaticVertexBufferObjectBox3 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectBox3);
    pStaticVertexBufferObjectBox3 = nullptr;
  }
  if (pStaticVertexBufferObjectCube3 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectCube3);
    pStaticVertexBufferObjectCube3 = nullptr;
  }
  if (pStaticVertexBufferObjectPlane3 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectPlane3);
    pStaticVertexBufferObjectPlane3 = nullptr;
  }

  if (pStaticVertexBufferObjectTeapot2 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectTeapot2);
    pStaticVertexBufferObjectTeapot2 = nullptr;
  }
  if (pStaticVertexBufferObjectSphere2 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectSphere2);
    pStaticVertexBufferObjectSphere2 = nullptr;
  }
  if (pStaticVertexBufferObjectBox2 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectBox2);
    pStaticVertexBufferObjectBox2 = nullptr;
  }
  if (pStaticVertexBufferObjectCube2 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectCube2);
    pStaticVertexBufferObjectCube2 = nullptr;
  }
  if (pStaticVertexBufferObjectPlane2 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectPlane2);
    pStaticVertexBufferObjectPlane2 = nullptr;
  }

  if (pStaticVertexBufferObjectGear0 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectGear0);
    pStaticVertexBufferObjectGear0 = nullptr;
  }
  if (pStaticVertexBufferObjectTeapot0 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectTeapot0);
    pStaticVertexBufferObjectTeapot0 = nullptr;
  }
  if (pStaticVertexBufferObjectSphere0 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectSphere0);
    pStaticVertexBufferObjectSphere0 = nullptr;
  }
  if (pStaticVertexBufferObjectBox0 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectBox0);
    pStaticVertexBufferObjectBox0 = nullptr;
  }
  if (pStaticVertexBufferObjectCube0 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectCube0);
    pStaticVertexBufferObjectCube0 = nullptr;
  }
  if (pStaticVertexBufferObjectPlane0 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectPlane0);
    pStaticVertexBufferObjectPlane0 = nullptr;
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


  if (pStaticVertexBufferObjectScreenRect != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectScreenRect);
    pStaticVertexBufferObjectScreenRect = nullptr;
  }

  if (pStaticVertexBufferObjectStatue != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectStatue);
    pStaticVertexBufferObjectStatue = nullptr;
  }

  if (pStaticVertexBufferObject != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObject);
    pStaticVertexBufferObject = nullptr;
  }

  if (pShaderScreenRect != nullptr) {
    pContext->DestroyShader(pShaderScreenRect);
    pShaderScreenRect = nullptr;
  }

  if (pShaderParallaxNormalMap != nullptr) {
    pContext->DestroyShader(pShaderParallaxNormalMap);
    pShaderParallaxNormalMap = nullptr;
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

  if (pTextureFrameBufferObject != nullptr) {
    pContext->DestroyTextureFrameBufferObject(pTextureFrameBufferObject);
    pTextureFrameBufferObject = nullptr;
  }

  pContext = nullptr;

  if (pWindow != nullptr) {
    system.DestroyWindow(pWindow);
    pWindow = nullptr;
  }
}

void cApplication::_OnWindowEvent(const opengl::cWindowEvent& event)
{
  std::cout<<"cApplication::_OnWindowEvent"<<std::endl;

  if (event.IsQuit()) {
    std::cout<<"cApplication::_OnWindowEvent Quiting"<<std::endl;
    bIsDone = true;
  }
}

void cApplication::_OnMouseEvent(const opengl::cMouseEvent& event)
{
  // These a little too numerous to log every single one
  //std::cout<<"cApplication::_OnMouseEvent"<<std::endl;

  if (event.IsMouseMove()) {
    //std::cout<<"cApplication::_OnMouseEvent Mouse move"<<std::endl;

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
  std::cout<<"cApplication::_OnKeyboardEvent"<<std::endl;
  if (event.IsKeyDown()) {
    switch (event.GetKeyCode()) {
      case SDLK_ESCAPE: {
        std::cout<<"cApplication::_OnKeyboardEvent Escape key pressed, quiting"<<std::endl;
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
        std::cout<<"cApplication::_OnKeyboardEvent spacebar down"<<std::endl;
        bIsRotating = false;
        break;
      }
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case SDLK_SPACE: {
        std::cout<<"cApplication::_OnKeyboardEvent spacebar up"<<std::endl;
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
  description.push_back("Esc quit");

  return description;
}

void cApplication::Run()
{
  std::cout<<"cApplication::Run"<<std::endl;

  assert(pContext != nullptr);
  assert(pContext->IsValid());
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
  assert(pShaderParallaxNormalMap != nullptr);
  assert(pShaderParallaxNormalMap->IsCompiledProgram());
  assert(pShaderScreenRect != nullptr);
  assert(pShaderScreenRect->IsCompiledProgram());
  assert(pStaticVertexBufferObject != nullptr);
  assert(pStaticVertexBufferObject->IsCompiled());
  assert(pStaticVertexBufferObjectStatue != nullptr);
  assert(pStaticVertexBufferObjectStatue->IsCompiled());
  assert(pStaticVertexBufferObjectScreenRect != nullptr);
  assert(pStaticVertexBufferObjectScreenRect->IsCompiled());


  assert(pShaderCrate != nullptr);
  assert(pShaderCrate->IsCompiledProgram());
  assert(pShaderFog != nullptr);
  assert(pShaderFog->IsCompiledProgram());
  assert(pShaderMetal != nullptr);
  assert(pShaderMetal->IsCompiledProgram());

  assert(pStaticVertexBufferObjectPlane0 != nullptr);
  assert(pStaticVertexBufferObjectPlane0->IsCompiled());
  assert(pStaticVertexBufferObjectCube0 != nullptr);
  assert(pStaticVertexBufferObjectCube0->IsCompiled());
  assert(pStaticVertexBufferObjectBox0 != nullptr);
  assert(pStaticVertexBufferObjectBox0->IsCompiled());
  assert(pStaticVertexBufferObjectSphere0 != nullptr);
  assert(pStaticVertexBufferObjectSphere0->IsCompiled());
  assert(pStaticVertexBufferObjectTeapot0 != nullptr);
  assert(pStaticVertexBufferObjectTeapot0->IsCompiled());
  //assert(pStaticVertexBufferObjectGear0 != nullptr);
  //assert(pStaticVertexBufferObjectGear0->IsCompiled());

  assert(pStaticVertexBufferObjectPlane2 != nullptr);
  assert(pStaticVertexBufferObjectPlane2->IsCompiled());
  assert(pStaticVertexBufferObjectCube2 != nullptr);
  assert(pStaticVertexBufferObjectCube2->IsCompiled());
  assert(pStaticVertexBufferObjectBox2 != nullptr);
  assert(pStaticVertexBufferObjectBox2->IsCompiled());
  assert(pStaticVertexBufferObjectSphere2 != nullptr);
  assert(pStaticVertexBufferObjectSphere2->IsCompiled());
  assert(pStaticVertexBufferObjectTeapot2 != nullptr);
  assert(pStaticVertexBufferObjectTeapot2->IsCompiled());

  assert(pStaticVertexBufferObjectPlane3 != nullptr);
  assert(pStaticVertexBufferObjectPlane3->IsCompiled());
  assert(pStaticVertexBufferObjectCube3 != nullptr);
  assert(pStaticVertexBufferObjectCube3->IsCompiled());
  assert(pStaticVertexBufferObjectBox3 != nullptr);
  assert(pStaticVertexBufferObjectBox3->IsCompiled());
  assert(pStaticVertexBufferObjectSphere3 != nullptr);
  assert(pStaticVertexBufferObjectSphere3->IsCompiled());
  assert(pStaticVertexBufferObjectTeapot3 != nullptr);
  assert(pStaticVertexBufferObjectTeapot3->IsCompiled());

  assert(pStaticVertexBufferObjectPointLight != nullptr);
  assert(pStaticVertexBufferObjectPointLight->IsCompiled());
  assert(pStaticVertexBufferObjectSpotLight != nullptr);
  assert(pStaticVertexBufferObjectSpotLight->IsCompiled());
  assert(pStaticVertexBufferObjectParallaxNormalMap != nullptr);
  assert(pStaticVertexBufferObjectParallaxNormalMap->IsCompiled());

  // Print the input instructions
  const std::vector<std::string> inputDescription = GetInputDescription();
  const size_t n = inputDescription.size();
  for (size_t i = 0; i < n; i++) std::cout<<inputDescription[i]<<std::endl;

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

  spitfire::math::cMat4 matTranslationStatue[9];
  i = 0;
  for (size_t y = 0; y < 3; y++) {
    for (size_t x = 0; x < 3; x++) {
      const spitfire::math::cVec3 position((-1.0f + float(x)) * fSpacingX, ((-2.0f - float(y)) * fSpacingY), 0.0f);
      matTranslationStatue[i].SetTranslation(position);
      i++;
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

  pContext->BindShader(*pShaderParallaxNormalMap);
    // Directional light
    pContext->SetShaderConstant("directionalLight.ambientColour", lightDirectionalAmbientColour);
    pContext->SetShaderConstant("directionalLight.diffuseColour", lightDirectionalDiffuseColour);
    pContext->SetShaderConstant("directionalLight.specularColour", lightDirectionalSpecularColour);

    // Setup materials
    pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
    pContext->SetShaderConstant("material.diffuseColour", materialDiffuseColour);
    pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
    pContext->SetShaderConstant("material.fShininess", fMaterialShininess);
  pContext->UnBindShader(*pShaderParallaxNormalMap);


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
      pWindow->UpdateEvents();

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
    pContext->BindShader(*pShaderParallaxNormalMap);
      pContext->SetShaderConstant("matView", matView);

      // Directional light
      pContext->SetShaderConstant("directionalLight.direction", lightDirection);
    pContext->UnBindShader(*pShaderParallaxNormalMap);

    {
      // Render the scene into the frame buffer object
      const spitfire::math::cColour clearColour(0.0f, 1.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObject);

      if (bIsWireframe) pContext->EnableWireframe();

      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);
      pContext->BindTextureCubeMap(2, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCubeMappedTeapot);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObject);

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(2, *pTextureCubeMap);
      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);

      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToTexture(*pTextureFrameBufferObject);
    }

    {
      // Render the scene with the new texture
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToScreen();

      if (bIsWireframe) pContext->EnableWireframe();


      // Render the teapot
      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);
      pContext->BindTextureCubeMap(2, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCubeMappedTeapot);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObject);

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(2, *pTextureCubeMap);
      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);


      // Render the statues
      {
        pContext->BindTexture(0, *pTextureMarble);

        pContext->BindShader(*pShaderLights);

        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectStatue);

        for (size_t i = 0; i < 9; i++) {
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationStatue[i] * matObjectRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectStatue);
        }

        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectStatue);

        pContext->UnBindShader(*pShaderLights);

        pContext->UnBindTexture(0, *pTextureMarble);
      }


      // Render the parallax normal map cube
      {
        pContext->BindTexture(0, *pTextureNormalMapDiffuse);
        pContext->BindTexture(1, *pTextureNormalMapSpecular);
        pContext->BindTexture(2, *pTextureNormalMapNormal);
        pContext->BindTexture(3, *pTextureNormalMapHeight);

        pContext->BindShader(*pShaderParallaxNormalMap);

        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectParallaxNormalMap);

        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationParallaxNormalMap);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectParallaxNormalMap);

        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectParallaxNormalMap);

        pContext->UnBindShader(*pShaderParallaxNormalMap);

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
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectPointLight);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTransform);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectPointLight);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectPointLight);
        }

        {
          pContext->SetShaderConstant("material.ambientColour", lightSpotColour);

          spitfire::math::cMat4 matTransform;
          matTransform.SetTranslation(lightSpotPosition);
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectSpotLight);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTransform);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectSpotLight);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectSpotLight);
        }

        pContext->UnBindShader(*pShaderMetal);
      }


      // Render the cubemapped objects
      pContext->BindShader(*pShaderMetal);

      {
        pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[0] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectPlane0);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane0);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectCube0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[1] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectCube0);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectCube0);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectBox0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[2] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectBox0);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectBox0);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[3] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectSphere0);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere0);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[4] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectTeapot0);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot0);
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
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[5] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectPlane3);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane3);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectCube3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[6] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectCube3);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectCube3);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectBox3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[7] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectBox3);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectBox3);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[8] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectSphere3);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere3);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[9] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectTeapot3);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot3);
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
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[10] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectPlane2);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane2);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectCube2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[11] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectCube2);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectCube2);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectBox2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[12] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectBox2);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectBox2);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[13] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectSphere2);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere2);
        }

        {
          pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[14] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectTeapot2);
          pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot2);
        }
      }

      pContext->UnBindShader(*pShaderFog);

      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);



      // Now draw an overlay of our rendered texture
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      // Move the rectangle into the bottom right hand corner of the screen
      spitfire::math::cMat4 matModelView2D;
      matModelView2D.SetTranslation(0.75f + (0.5f * 0.25f), 0.75f + (0.5f * 0.25f), 0.0f);

      pContext->BindTexture(0, *pTextureFrameBufferObject);

      pContext->BindShader(*pShaderScreenRect);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenRect);

      {
        pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

        pContext->DrawStaticVertexBufferObjectTriangles2D(*pStaticVertexBufferObjectScreenRect);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenRect);

      pContext->UnBindShader(*pShaderScreenRect);

      pContext->UnBindTexture(0, *pTextureFrameBufferObject);

      pContext->EndRenderMode2D();

      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToScreen();
    }

    // Gather our frames per second
    Frames++;
    {
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 5000) {
        float seconds = (t - T0) / 1000.0f;
        float fps = Frames / seconds;
        std::cout<<Frames<<" frames in "<<seconds<<" seconds = "<<fps<<" FPS"<<std::endl;
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


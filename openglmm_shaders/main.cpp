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

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>

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
  template <class T>
  void CreatePlane(T* pObject, size_t nTextureCoordinates);
  template <class T>
  void CreateCube(T* pObject, size_t nTextureCoordinates);
  template <class T>
  void CreateBox(T* pObject, size_t nTextureCoordinates);
  template <class T>
  void CreateSphere(T* pObject, size_t nTextureCoordinates);
  template <class T>
  void CreateTeapot(T* pObject, size_t nTextureCoordinates);
  template <class T>
  void CreateGear(T* pObject);

  void CreateBoxVBO();
  void CreateScreenRectVBO();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  std::vector<std::string> GetInputDescription() const;

  bool bIsMovingForward;
  bool bIsMovingLeft;
  bool bIsMovingRight;
  bool bIsMovingBackward;

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

  opengl::cShader* pShaderCubeMap;
  opengl::cShader* pShaderScreenRect;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObject;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectScreenRect;


  opengl::cShader* pShaderCrate;
  opengl::cShader* pShaderMetal;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPlane0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectCube0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBox0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectSphere0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectTeapot0;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectGear0;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectPlane3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectCube3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectBox3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectSphere3;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectTeapot3;
};

cApplication::cApplication() :
  bIsMovingForward(false),
  bIsMovingLeft(false),
  bIsMovingRight(false),
  bIsMovingBackward(false),

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

  pShaderCubeMap(nullptr),
  pShaderScreenRect(nullptr),

  pStaticVertexBufferObject(nullptr),
  pStaticVertexBufferObjectScreenRect(nullptr),


  pShaderCrate(nullptr),
  pShaderMetal(nullptr),

  pStaticVertexBufferObjectPlane0(nullptr),
  pStaticVertexBufferObjectCube0(nullptr),
  pStaticVertexBufferObjectBox0(nullptr),
  pStaticVertexBufferObjectSphere0(nullptr),
  pStaticVertexBufferObjectTeapot0(nullptr),
  pStaticVertexBufferObjectGear0(nullptr),

  pStaticVertexBufferObjectPlane3(nullptr),
  pStaticVertexBufferObjectCube3(nullptr),
  pStaticVertexBufferObjectBox3(nullptr),
  pStaticVertexBufferObjectSphere3(nullptr),
  pStaticVertexBufferObjectTeapot3(nullptr)
{
}

cApplication::~cApplication()
{
  Destroy();
}

template <class T>
void cApplication::CreatePlane(T* pObject, size_t nTextureCoordinates)
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

template <class T>
void cApplication::CreateCube(T* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateCube(fWidth, *pGeometryDataPtr, nTextureCoordinates);

  pObject->SetData(pGeometryDataPtr);

  pObject->Compile(system);
}

template <class T>
void cApplication::CreateBox(T* pObject, size_t nTextureCoordinates)
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

template <class T>
void cApplication::CreateSphere(T* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fRadius = 1.0f;
  const size_t nSegments = 20;

  opengl::cGeometryBuilder builder;
  builder.CreateSphere(fRadius, nSegments, *pGeometryDataPtr, nTextureCoordinates);

  pObject->SetData(pGeometryDataPtr);

  pObject->Compile(system);
}

template <class T>
void cApplication::CreateTeapot(T* pObject, size_t nTextureCoordinates)
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

template <class T>
void cApplication::CreateGear(T* pObject)
{
}

void cApplication::CreateBoxVBO()
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

  pShaderCubeMap = pContext->CreateShader(TEXT("shaders/cubemap.vert"), TEXT("shaders/cubemap.frag"));
  assert(pShaderCubeMap != nullptr);

  pShaderScreenRect = pContext->CreateShader(TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(pShaderScreenRect != nullptr);

  pStaticVertexBufferObject = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObject != nullptr);
  CreateBoxVBO();

  pStaticVertexBufferObjectScreenRect = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectScreenRect != nullptr);
  CreateScreenRectVBO();


  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  pShaderMetal = pContext->CreateShader(TEXT("shaders/metal.vert"), TEXT("shaders/metal.frag"));

  pStaticVertexBufferObjectPlane0 = pContext->CreateStaticVertexBufferObject();
  CreatePlane(pStaticVertexBufferObjectPlane0, 0);
  pStaticVertexBufferObjectCube0 = pContext->CreateStaticVertexBufferObject();
  CreateCube(pStaticVertexBufferObjectCube0, 0);
  pStaticVertexBufferObjectBox0 = pContext->CreateStaticVertexBufferObject();
  CreateBox(pStaticVertexBufferObjectBox0, 0);
  pStaticVertexBufferObjectSphere0 = pContext->CreateStaticVertexBufferObject();
  CreateSphere(pStaticVertexBufferObjectSphere0, 0);
  pStaticVertexBufferObjectTeapot0 = pContext->CreateStaticVertexBufferObject();
  CreateTeapot(pStaticVertexBufferObjectTeapot0, 0);
  pStaticVertexBufferObjectGear0 = pContext->CreateStaticVertexBufferObject();
  CreateGear(pStaticVertexBufferObjectGear0);

  pStaticVertexBufferObjectPlane3 = pContext->CreateStaticVertexBufferObject();
  CreatePlane(pStaticVertexBufferObjectPlane3, 3);
  pStaticVertexBufferObjectCube3 = pContext->CreateStaticVertexBufferObject();
  CreateCube(pStaticVertexBufferObjectCube3, 3);
  pStaticVertexBufferObjectBox3 = pContext->CreateStaticVertexBufferObject();
  CreateBox(pStaticVertexBufferObjectBox3, 3);
  pStaticVertexBufferObjectSphere3 = pContext->CreateStaticVertexBufferObject();
  CreateSphere(pStaticVertexBufferObjectSphere3, 3);
  pStaticVertexBufferObjectTeapot3 = pContext->CreateStaticVertexBufferObject();
  CreateTeapot(pStaticVertexBufferObjectTeapot3, 3);


  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
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
  if (pShaderCrate != nullptr) {
    pContext->DestroyShader(pShaderCrate);
    pShaderCrate = nullptr;
  }


  if (pStaticVertexBufferObjectScreenRect != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectScreenRect);
    pStaticVertexBufferObjectScreenRect = nullptr;
  }

  if (pStaticVertexBufferObject != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObject);
    pStaticVertexBufferObject = nullptr;
  }

  if (pShaderScreenRect != nullptr) {
    pContext->DestroyShader(pShaderScreenRect);
    pShaderScreenRect = nullptr;
  }

  if (pShaderCubeMap != nullptr) {
    pContext->DestroyShader(pShaderCubeMap);
    pShaderCubeMap = nullptr;
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
    std::cout<<"cApplication::_OnMouseEvent Mouse move"<<std::endl;

    if (fabs(event.GetX() - (pWindow->GetWidth() * 0.5f)) > 0.5f) {
      camera.RotateZ(-0.1f * (event.GetX() - (pWindow->GetWidth() * 0.5f)));
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
  assert(pShaderCubeMap != nullptr);
  assert(pShaderCubeMap->IsCompiledProgram());
  assert(pShaderScreenRect != nullptr);
  assert(pShaderScreenRect->IsCompiledProgram());
  assert(pStaticVertexBufferObject != nullptr);
  assert(pStaticVertexBufferObject->IsCompiled());
  assert(pStaticVertexBufferObjectScreenRect != nullptr);
  assert(pStaticVertexBufferObjectScreenRect->IsCompiled());


  assert(pShaderCrate != nullptr);
  assert(pShaderCrate->IsCompiledProgram());
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

  // Print the input instructions
  const std::vector<std::string> inputDescription = GetInputDescription();
  const size_t n = inputDescription.size();
  for (size_t i = 0; i < n; i++) std::cout<<inputDescription[i]<<std::endl;

  // Set up the camera
  camera.SetPosition(spitfire::math::cVec3(0.0f, 10.0f, 5.0f));
  spitfire::math::cQuaternion cameraRotation;
  cameraRotation.SetFromAxisAngle(spitfire::math::v3Up, -45.0f);
  camera.SetRotation(cameraRotation);

  // Set up the translations for our objects
  const size_t columns = 5;
  const size_t rows = 2;

  const float fSpacingX = 0.007f * pContext->GetWidth() / float(rows);
  const float fSpacingY = 0.03f * pContext->GetHeight() / float(columns);
  const float fLeft = -0.5f * float(columns - 1) * fSpacingX;
  const float fDepth = -0.5f * float(rows - 1) * fSpacingY;

  spitfire::math::cVec3 positions[columns * rows];
  size_t i = 0;
  for (size_t y = 0; y < rows; y++) {
    for (size_t x = 0; x < columns; x++) {
      positions[i].Set(fLeft + (x * fSpacingX), fDepth + (y * fSpacingY), 0.0f);
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

  uint32_t T0 = 0;
  uint32_t Frames = 0;

  uint32_t previousTime = SDL_GetTicks();
  uint32_t currentTime = SDL_GetTicks();

  pContext->EnableLighting();


  // Light
  const spitfire::math::cVec3 lightPosition(5.0f, 5.0f, 10.0f);
  const spitfire::math::cColour lightAmbientColour(0.2f, 0.2f, 0.2f);
  const spitfire::math::cColour lightDiffuseColour(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour lightSpecularColour(1.0f, 1.0f, 1.0f);

  // Material
  const spitfire::math::cColour materialDiffuseColour(0.5f, 0.5f, 0.5f);
  const spitfire::math::cColour materialAmbientColour(0.0f, 0.0f, 0.0f);
  const spitfire::math::cColour materialSpecularColour(1.0f, 1.0f, 1.0f);
  const float fMaterialShininess = 50.0f;

  // Set our shader constants
  pContext->BindShader(*pShaderMetal);

    // Setup lighting
    pContext->SetShaderConstant("light.ambientColour", lightAmbientColour);
    pContext->SetShaderConstant("light.diffuseColour", lightDiffuseColour);
    pContext->SetShaderConstant("light.specularColour", lightSpecularColour);

    // Setup materials
    pContext->SetShaderConstant("material.diffuseColour", materialDiffuseColour);
    pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
    pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
    pContext->SetShaderConstant("material.fShininess", fMaterialShininess);

  pContext->UnBindShader(*pShaderMetal);


  // Setup mouse
  pWindow->ShowCursor(false);
  pWindow->WarpCursorToMiddleOfScreen();

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();

    // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
    pWindow->WarpCursorToMiddleOfScreen();

    // Update state
    previousTime = currentTime;
    currentTime = SDL_GetTicks();

    // Update the camera
    const float fDistance = 0.01f;
    if (bIsMovingForward) camera.MoveY(fDistance);
    if (bIsMovingBackward) camera.MoveY(-fDistance);
    if (bIsMovingLeft) camera.MoveX(-fDistance);
    if (bIsMovingRight) camera.MoveX(fDistance);

    const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

    const spitfire::math::cMat4 matView = camera.CalculateViewMatrix();

    // Set up the metal shader
    pContext->BindShader(*pShaderMetal);
      pContext->SetShaderConstant("light.position", matView * lightPosition);
    pContext->UnBindShader(*pShaderMetal);

    // Set up the cube map shader
    pContext->BindShader(*pShaderCubeMap);
      pContext->SetShaderConstant("cameraPosition", matView * spitfire::math::cVec3(0.0f, 0.0f, 0.0f));
    pContext->UnBindShader(*pShaderCubeMap);


    // Update object rotation
    if (bIsRotating) fAngleRadians += float(currentTime - previousTime) * fRotationSpeed;

    rotation.SetFromAxisAngle(spitfire::math::v3Up, fAngleRadians);

    matObjectRotation.SetRotation(rotation);


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
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView);

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

      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);
      pContext->BindTextureCubeMap(2, *pTextureCubeMap);

      pContext->BindShader(*pShaderCubeMap);

      pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObject);

      pContext->UnBindShader(*pShaderCubeMap);

      pContext->UnBindTextureCubeMap(2, *pTextureCubeMap);
      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);



      pContext->BindShader(*pShaderMetal);

      {
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


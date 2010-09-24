#include <cassert>
#include <cmath>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>

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
#include <libopenglmm/cVertexArray.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

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

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  bool bIsRotating;
  bool bIsWireframe;
  bool bIsUsingVertexBufferObjects;
  bool bIsDone;

  opengl::cSystem system;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  opengl::cTexture* pTextureDiffuse;
  opengl::cTexture* pTextureLightMap;
  opengl::cTexture* pTextureDetail;

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

  opengl::cDynamicVertexArray* pDynamicVertexArrayPlane0;
  opengl::cDynamicVertexArray* pDynamicVertexArrayCube0;
  opengl::cDynamicVertexArray* pDynamicVertexArrayBox0;
  opengl::cDynamicVertexArray* pDynamicVertexArraySphere0;
  opengl::cDynamicVertexArray* pDynamicVertexArrayTeapot0;
  opengl::cDynamicVertexArray* pDynamicVertexArrayGear0;

  opengl::cDynamicVertexArray* pDynamicVertexArrayPlane3;
  opengl::cDynamicVertexArray* pDynamicVertexArrayCube3;
  opengl::cDynamicVertexArray* pDynamicVertexArrayBox3;
  opengl::cDynamicVertexArray* pDynamicVertexArraySphere3;
  opengl::cDynamicVertexArray* pDynamicVertexArrayTeapot3;
};

cApplication::cApplication() :
  bIsRotating(true),
  bIsWireframe(false),
  bIsUsingVertexBufferObjects(true),
  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  pTextureDiffuse(nullptr),
  pTextureLightMap(nullptr),
  pTextureDetail(nullptr),

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
  pStaticVertexBufferObjectTeapot3(nullptr),

  pDynamicVertexArrayPlane0(nullptr),
  pDynamicVertexArrayCube0(nullptr),
  pDynamicVertexArrayBox0(nullptr),
  pDynamicVertexArraySphere0(nullptr),
  pDynamicVertexArrayTeapot0(nullptr),
  pDynamicVertexArrayGear0(nullptr),

  pDynamicVertexArrayPlane3(nullptr),
  pDynamicVertexArrayCube3(nullptr),
  pDynamicVertexArrayBox3(nullptr),
  pDynamicVertexArraySphere3(nullptr),
  pDynamicVertexArrayTeapot3(nullptr)
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

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoordinates;
  //std::vector<uint16_t> indices;

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreatePlane(fWidth, fDepth, vertices, normals, textureCoordinates, nTextureCoordinates);

  pObject->SetVertices(vertices);
  pObject->SetNormals(normals);
  if (nTextureCoordinates != 0) pObject->SetTextureCoordinates(textureCoordinates);
  //pObject->SetIndices(indices);

  pObject->Compile(system);
}

template <class T>
void cApplication::CreateCube(T* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoordinates;
  //std::vector<uint16_t> indices;

  const float fWidth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateCube(fWidth, vertices, normals, textureCoordinates, nTextureCoordinates);

  pObject->SetVertices(vertices);
  pObject->SetNormals(normals);
  if (nTextureCoordinates != 0) pObject->SetTextureCoordinates(textureCoordinates);
  //pObject->SetIndices(indices);

  pObject->Compile(system);
}

template <class T>
void cApplication::CreateBox(T* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoordinates;
  //std::vector<uint16_t> indices;

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;
  const float fHeight = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateBox(fWidth, fDepth, fHeight, vertices, normals, textureCoordinates, nTextureCoordinates);

  pObject->SetVertices(vertices);
  pObject->SetNormals(normals);
  if (nTextureCoordinates != 0) pObject->SetTextureCoordinates(textureCoordinates);
  //pObject->SetIndices(indices);

  pObject->Compile(system);
}

template <class T>
void cApplication::CreateSphere(T* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoordinates;
  //std::vector<uint16_t> indices;

  const float fRadius = 1.0f;
  const size_t nSegments = 20;

  opengl::cGeometryBuilder builder;
  builder.CreateSphere(fRadius, nSegments, vertices, normals, textureCoordinates, nTextureCoordinates);

  pObject->SetVertices(vertices);
  pObject->SetNormals(normals);
  if (nTextureCoordinates != 0) pObject->SetTextureCoordinates(textureCoordinates);
  //pObject->SetIndices(indices);

  pObject->Compile(system);
}

template <class T>
void cApplication::CreateTeapot(T* pObject, size_t nTextureCoordinates)
{
  assert(pObject != nullptr);

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoordinates;
  std::vector<uint16_t> indices;

  const float fRadius = 1.0f;
  const size_t nSegments = 20;

  opengl::cGeometryBuilder builder;
  builder.CreateTeapot(fRadius, nSegments, vertices, normals, textureCoordinates, nTextureCoordinates, indices);

  pObject->SetVertices(vertices);
  pObject->SetNormals(normals);
  if (nTextureCoordinates != 0) pObject->SetTextureCoordinates(textureCoordinates);
  pObject->SetIndices(indices);

  pObject->Compile(system);
}

template <class T>
void cApplication::CreateGear(T* pObject)
{
}

bool cApplication::Create()
{
  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  opengl::cResolution resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 1024) || (resolution.height < 768) || (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8)) {
    std::cout<<"Current screen resolution is not adequate "<<resolution.width<<"x"<<resolution.height<<std::endl;
    return false;
  }

  // Set our required resolution
  resolution.width = 1024;
  resolution.height = 768;
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  pWindow = system.CreateWindow(TEXT("openglmm_geometry"), resolution, false);
  if (pWindow == nullptr) {
    std::cout<<"Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }

  pTextureDiffuse = pContext->CreateTexture(TEXT("textures/diffuse.png"));
  pTextureLightMap = pContext->CreateTexture(TEXT("textures/lightmap.png"));
  pTextureDetail = pContext->CreateTexture(TEXT("textures/detail.png"));

  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  pShaderCrate->bTexUnit0 = true;
  pShaderCrate->bTexUnit1 = true;
  pShaderCrate->bTexUnit2 = true;

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

  pDynamicVertexArrayPlane0 = pContext->CreateDynamicVertexArray();
  CreatePlane(pDynamicVertexArrayPlane0, 0);
  pDynamicVertexArrayCube0 = pContext->CreateDynamicVertexArray();
  CreateCube(pDynamicVertexArrayCube0, 0);
  pDynamicVertexArrayBox0 = pContext->CreateDynamicVertexArray();
  CreateBox(pDynamicVertexArrayBox0, 0);
  pDynamicVertexArraySphere0 = pContext->CreateDynamicVertexArray();
  CreateSphere(pDynamicVertexArraySphere0, 0);
  pDynamicVertexArrayTeapot0 = pContext->CreateDynamicVertexArray();
  CreateTeapot(pDynamicVertexArrayTeapot0, 0);
  pDynamicVertexArrayGear0 = pContext->CreateDynamicVertexArray();
  CreateGear(pDynamicVertexArrayGear0);

  pDynamicVertexArrayPlane3 = pContext->CreateDynamicVertexArray();
  CreatePlane(pDynamicVertexArrayPlane3, 3);
  pDynamicVertexArrayCube3 = pContext->CreateDynamicVertexArray();
  CreateCube(pDynamicVertexArrayCube3, 3);
  pDynamicVertexArrayBox3 = pContext->CreateDynamicVertexArray();
  CreateBox(pDynamicVertexArrayBox3, 3);
  pDynamicVertexArraySphere3 = pContext->CreateDynamicVertexArray();
  CreateSphere(pDynamicVertexArraySphere3, 3);
  pDynamicVertexArrayTeapot3 = pContext->CreateDynamicVertexArray();
  CreateTeapot(pDynamicVertexArrayTeapot3, 3);

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  if (pDynamicVertexArrayTeapot3 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayTeapot3);
    pDynamicVertexArrayTeapot3 = nullptr;
  }
  if (pDynamicVertexArraySphere3 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArraySphere3);
    pDynamicVertexArraySphere3 = nullptr;
  }
  if (pDynamicVertexArrayBox3 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayBox3);
    pDynamicVertexArrayBox3 = nullptr;
  }
  if (pDynamicVertexArrayCube3 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayCube3);
    pDynamicVertexArrayCube3 = nullptr;
  }
  if (pDynamicVertexArrayPlane3 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayPlane3);
    pDynamicVertexArrayPlane3 = nullptr;
  }

  if (pDynamicVertexArrayGear0 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayGear0);
    pDynamicVertexArrayGear0 = nullptr;
  }
  if (pDynamicVertexArrayTeapot0 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayTeapot0);
    pDynamicVertexArrayTeapot0 = nullptr;
  }
  if (pDynamicVertexArraySphere0 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArraySphere0);
    pDynamicVertexArraySphere0 = nullptr;
  }
  if (pDynamicVertexArrayBox0 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayBox0);
    pDynamicVertexArrayBox0 = nullptr;
  }
  if (pDynamicVertexArrayCube0 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayCube0);
    pDynamicVertexArrayCube0 = nullptr;
  }
  if (pDynamicVertexArrayPlane0 != nullptr) {
    pContext->DestroyDynamicVertexArray(pDynamicVertexArrayPlane0);
    pDynamicVertexArrayPlane0 = nullptr;
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
}

void cApplication::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  std::cout<<"cApplication::_OnKeyboardEvent"<<std::endl;
  if (event.IsKeyDown()) {
    switch (event.GetKeyCode()) {
      case SDLK_ESCAPE: {
        std::cout<<"cApplication::_OnKeyboardEvent F1 key pressed, quiting"<<std::endl;
        bIsDone = true;
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
        bIsWireframe = !bIsWireframe;
        std::cout<<"cApplication::_OnKeyboardEvent Switching "<<(bIsWireframe ? "to" : "from")<<" wireframe"<<std::endl;
        break;
      }
      case SDLK_v: {
        bIsUsingVertexBufferObjects = !bIsUsingVertexBufferObjects;
        std::cout<<"cApplication::_OnKeyboardEvent Switching to "<<(bIsUsingVertexBufferObjects ? "vertex buffer objects" : "vertex arrays")<<std::endl;
        break;
      }
    }
  }
}

void cApplication::Run()
{
  assert(pContext != nullptr);
  assert(pContext->IsValid());
  assert(pTextureDiffuse != nullptr);
  assert(pTextureDiffuse->IsValid());
  assert(pTextureLightMap != nullptr);
  assert(pTextureLightMap->IsValid());
  assert(pTextureDetail != nullptr);
  assert(pTextureDetail->IsValid());
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

  assert(pDynamicVertexArrayPlane0 != nullptr);
  assert(pDynamicVertexArrayPlane0->IsCompiled());
  assert(pDynamicVertexArrayCube0 != nullptr);
  assert(pDynamicVertexArrayCube0->IsCompiled());
  assert(pDynamicVertexArrayBox0 != nullptr);
  assert(pDynamicVertexArrayBox0->IsCompiled());
  assert(pDynamicVertexArraySphere0 != nullptr);
  assert(pDynamicVertexArraySphere0->IsCompiled());
  assert(pDynamicVertexArrayTeapot0 != nullptr);
  assert(pDynamicVertexArrayTeapot0->IsCompiled());
  //assert(pDynamicVertexArrayGear0 != nullptr);
  //assert(pDynamicVertexArrayGear0->IsCompiled());

  assert(pDynamicVertexArrayPlane3 != nullptr);
  assert(pDynamicVertexArrayPlane3->IsCompiled());
  assert(pDynamicVertexArrayCube3 != nullptr);
  assert(pDynamicVertexArrayCube3->IsCompiled());
  assert(pDynamicVertexArrayBox3 != nullptr);
  assert(pDynamicVertexArrayBox3->IsCompiled());
  assert(pDynamicVertexArraySphere3 != nullptr);
  assert(pDynamicVertexArraySphere3->IsCompiled());
  assert(pDynamicVertexArrayTeapot3 != nullptr);
  assert(pDynamicVertexArrayTeapot3->IsCompiled());

  const spitfire::math::cColour sunColour(0.2, 0.2, 0.0);

  // Setup materials
  const spitfire::math::cColour ambient(sunColour);
  pContext->SetMaterialAmbientColour(ambient);
  const spitfire::math::cColour diffuse(0.8, 0.1, 0.0);
  pContext->SetMaterialDiffuseColour(diffuse);
  const spitfire::math::cColour specular(1.0, 0.3, 0.3);
  pContext->SetMaterialSpecularColour(specular);
  const float fShininess = 50.0f;
  pContext->SetMaterialShininess(fShininess);


  // Setup lighting
  pContext->EnableLighting();
  pContext->EnableLight(0);
  const spitfire::math::cVec3 lightPosition(5.0f, 5.0f, 10.0f);
  pContext->SetLightPosition(0, lightPosition);
  const spitfire::math::cColour lightAmbient(sunColour);
  pContext->SetLightAmbientColour(0, lightAmbient);
  const spitfire::math::cColour lightDiffuse(1.0, 1.0, 1.0);
  pContext->SetLightDiffuseColour(0, lightDiffuse);
  const spitfire::math::cColour lightSpecular(1.0f, 1.0f, 1.0f);
  pContext->SetLightSpecularColour(0, lightSpecular);


  const size_t columns = 5;
  const size_t rows = 2;

  const float fSpacingX = 0.007f * pContext->GetWidth() / float(rows);
  const float fSpacingZ = 0.015f * pContext->GetHeight() / float(columns);
  const float fLeft = -0.5f * float(columns - 1) * fSpacingX;
  const float fTop = -0.5f * float(rows - 1) * fSpacingZ;

  spitfire::math::cVec3 positions[columns * rows];
  size_t i = 0;
  for (size_t z = 0; z < rows; z++) {
    for (size_t x = 0; x < columns; x++) {
      positions[i].Set(fLeft + (x * fSpacingX), 0, fTop + (z * fSpacingZ));
      i++;
    }
  }

  spitfire::math::cMat4 matTranslation[columns * rows];
  for (size_t i = 0; i < columns * rows; i++) matTranslation[i].SetTranslation(positions[i]);



  spitfire::math::cQuaternion rotation;
  spitfire::math::cMat4 matRotation;

  const spitfire::math::cVec3 axis(0.0f, 0.0f, 1.0f);
  float fAngleRadians = 0.0f;
  const float fRotationSpeed = 0.001f;

  uint32_t T0 = 0;
  uint32_t Frames = 0;

  uint32_t previousTime = SDL_GetTicks();
  uint32_t currentTime = SDL_GetTicks();

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();


    // Update state
    previousTime = currentTime;
    currentTime = SDL_GetTicks();

    if (bIsRotating) fAngleRadians += float(currentTime - previousTime) * fRotationSpeed;

    rotation.SetFromAxisAngle(axis, fAngleRadians);

    matRotation.SetRotation(rotation);


    // Render the scene
    const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
    pContext->SetClearColour(clearColour);

    pContext->BeginRendering();

    if (bIsWireframe) pContext->EnableWireframe();

    const spitfire::math::cVec3 position;
    const spitfire::math::cVec3 target(position);
    const spitfire::math::cVec3 eye(position + spitfire::math::cVec3(0.0f, -16.0f, 4.0f));
    spitfire::math::cMat4 matModelView;
    matModelView.LookAt(eye, target, spitfire::math::v3Up);

    pContext->BindShader(*pShaderMetal);

    if (bIsUsingVertexBufferObjects) {
      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[0] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectPlane0);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane0);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectCube0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[1] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectCube0);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectCube0);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectBox0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[2] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectBox0);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectBox0);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[3] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectSphere0);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere0);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[4] * matRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectTeapot0);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot0);
      }
    } else {
      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayPlane0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[0] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArrayPlane0);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayPlane0);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayCube0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[1] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArrayCube0);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayCube0);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayBox0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[2] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArrayBox0);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayBox0);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArraySphere0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[3] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArraySphere0);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArraySphere0);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayTeapot0);
          pContext->SetModelViewMatrix(matModelView * matTranslation[4] * matRotation);
          pContext->DrawDynamicVertexArrayTriangles(*pDynamicVertexArrayTeapot0);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayTeapot0);
      }
    }

    pContext->UnBindShader(*pShaderMetal);


    pContext->BindTexture(0, *pTextureDiffuse);
    pContext->BindTexture(1, *pTextureLightMap);
    pContext->BindTexture(2, *pTextureDetail);

    pContext->BindShader(*pShaderCrate);

    if (bIsUsingVertexBufferObjects) {
      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[5] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectPlane3);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectPlane3);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectCube3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[6] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectCube3);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectCube3);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectBox3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[7] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectBox3);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectBox3);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[8] * matRotation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectSphere3);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectSphere3);
      }

      {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[9] * matRotation);
          pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectTeapot3);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectTeapot3);
      }
    } else {
      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayPlane3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[5] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArrayPlane3);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayPlane3);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayCube3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[6] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArrayCube3);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayCube3);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayBox3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[7] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArrayBox3);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayBox3);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArraySphere3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[8] * matRotation);
          pContext->DrawDynamicVertexArrayQuads(*pDynamicVertexArraySphere3);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArraySphere3);
      }

      {
        pContext->BindDynamicVertexArray(*pDynamicVertexArrayTeapot3);
          pContext->SetModelViewMatrix(matModelView * matTranslation[9] * matRotation);
          pContext->DrawDynamicVertexArrayTriangles(*pDynamicVertexArrayTeapot3);
        pContext->UnBindDynamicVertexArray(*pDynamicVertexArrayTeapot3);
      }
    }

    pContext->UnBindShader(*pShaderCrate);

    pContext->UnBindTexture(2, *pTextureDetail);
    pContext->UnBindTexture(1, *pTextureLightMap);
    pContext->UnBindTexture(0, *pTextureDiffuse);

    pContext->EndRendering();

    // Gather our frames per second
    Frames++;
    {
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 5000) {
        float seconds = (t - T0) / 1000.0;
        float fps = Frames / seconds;
        std::cout<<Frames<<" frames in "<<seconds<<" seconds = "<<fps<<" FPS"<<std::endl;
        T0 = t;
        Frames = 0;
      }
    }
  };
}

int main(int argc, char** argv)
{
  bool bIsSuccess = true;

  cApplication application;

  bIsSuccess = application.Create();
  if (bIsSuccess) application.Run();

  application.Destroy();

  return bIsSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}

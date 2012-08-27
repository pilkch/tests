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
//#include <GL/GLee.h>
//#include <GL/glu.h>

// SDL headers
//#include <SDL/SDL_image.h>

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

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();
  ~cApplication();

  bool Create();
  void Destroy();

  void Run();

private:
  void CreateGear(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, float fInnerRadius, float fOuterRadius, float fWidth, size_t nTeeth, float fToothDepth);
  void CreateSphere(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, float fRadius, size_t nSegments);

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  bool bIsDone;

  opengl::cSystem system;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  opengl::cShader* pShader;

  opengl::cStaticVertexBufferObject* pGearVBO1;
  opengl::cStaticVertexBufferObject* pGearVBO2;
  opengl::cStaticVertexBufferObject* pGearVBO3;
};

cApplication::cApplication() :
  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  pShader(nullptr),

  pGearVBO1(nullptr),
  pGearVBO2(nullptr),
  pGearVBO3(nullptr)
{
}

cApplication::~cApplication()
{
  Destroy();
}

bool cApplication::Create()
{
  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  opengl::cResolution resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 300) || (resolution.height < 300) || (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8)) {
    std::cout<<"Current screen resolution is not adequate "<<resolution.width<<"x"<<resolution.height<<std::endl;
    return false;
  }

  // Set our required resolution
  resolution.width = 300;
  resolution.height = 300;
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  pWindow = system.CreateWindow(TEXT("openglmm_gears"), resolution, false);
  if (pWindow == nullptr) {
    std::cout<<"Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }

  pShader = pContext->CreateShader(TEXT("shaders/gear.vert"), TEXT("shaders/gear.frag"));
  if (pShader == nullptr) {
    std::cout<<"Shader could not be created"<<std::endl;
    return false;
  }

  pGearVBO1 = pContext->CreateStaticVertexBufferObject();
  pGearVBO2 = pContext->CreateStaticVertexBufferObject();
  pGearVBO3 = pContext->CreateStaticVertexBufferObject();
  if ((pGearVBO1 == nullptr) || (pGearVBO2 == nullptr) || (pGearVBO3 == nullptr)) {
    std::cout<<"VBO could not be created"<<std::endl;
    return false;
  }

  // Make the gears
  CreateGear(pGearVBO1, 1.0f, 4.0f, 1.0f, 20, 0.7f);
  CreateGear(pGearVBO2, 0.5f, 2.0f, 2.0f, 10, 0.7f);
  CreateGear(pGearVBO3, 1.3f, 2.0f, 0.5f, 10, 0.7f);
  //CreateSphere(pGearVBO3, 1.3f, 32);

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  if (pGearVBO3 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pGearVBO3);
    pGearVBO3 = nullptr;
  }
  if (pGearVBO2 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pGearVBO2);
    pGearVBO2 = nullptr;
  }
  if (pGearVBO1 != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pGearVBO1);
    pGearVBO1 = nullptr;
  }

  if (pShader != nullptr) {
    pContext->DestroyShader(pShader);
    pShader = nullptr;
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
    if (event.GetKeyCode() == SDLK_ESCAPE) {
      std::cout<<"cApplication::_OnKeyboardEvent Escape key pressed, quiting"<<std::endl;
      bIsDone = true;
    }
  }
}


//
// Build a gear wheel vbo object for rendering later.
//
// Input: fInnerRadius - radius of hole at center
//        fOuterRadius - radius at center of teeth
//        fWidth - width of gear
//        nTeeth - number of teeth
//        fToothDepth - depth of tooth
//
// NOTE: I haven't thought too much about the normals for each vertex, it doesn't look terrible so that is good enough for me
//

void cApplication::CreateGear(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, float fInnerRadius, float fOuterRadius, float fWidth, size_t nTeeth, float fToothDepth)
{
  std::vector<float> vertices;
  std::vector<float> normals;

  opengl::cGeometryBuilder builder;
  builder.CreateGear(fInnerRadius, fOuterRadius, fWidth, nTeeth, fToothDepth, vertices, normals);

  std::vector<float> data;
  const size_t n = vertices.size();
  for (size_t i = 0; i < n; i += 3) {
    data.push_back(vertices[i]);
    data.push_back(vertices[i + 1]);
    data.push_back(vertices[i + 2]);
    data.push_back(normals[i]);
    data.push_back(normals[i + 1]);
    data.push_back(normals[i + 2]);
  }
  opengl::cBufferSizes bufferSizes;
  bufferSizes.nVertexCount = vertices.size() / 3;
  bufferSizes.nVerticesPerPoint = 3;
  bufferSizes.nNormalsPerPoint = 3;
  pStaticVertexBufferObject->SetData(data, bufferSizes);

  pStaticVertexBufferObject->Compile(system);
}


//
// Build a sphere vbo object for rendering later.
//

void cApplication::CreateSphere(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, float fRadius, size_t nSegments)
{
  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoords;
  const size_t nTextureUnits = 0;

  opengl::cGeometryBuilder builder;
  builder.CreateSphere(fRadius, nSegments, vertices, normals, textureCoords, nTextureUnits);

  std::vector<float> data;
  const size_t n = vertices.size();
  for (size_t i = 0; i < n; i += 3) {
    data.push_back(vertices[i]);
    data.push_back(vertices[i + 1]);
    data.push_back(vertices[i + 2]);
    data.push_back(normals[i]);
    data.push_back(normals[i + 1]);
    data.push_back(normals[i + 2]);
  }
  opengl::cBufferSizes bufferSizes;
  bufferSizes.nVertexCount = vertices.size() / 3;
  bufferSizes.nVerticesPerPoint = 3;
  bufferSizes.nNormalsPerPoint = 3;
  pStaticVertexBufferObject->SetData(data, bufferSizes);

  pStaticVertexBufferObject->Compile(system);
}

void cApplication::Run()
{
  assert(pContext != nullptr);
  assert(pContext->IsValid());

  const spitfire::math::cColour ambientColour(0.1f, 0.1f, 0.1f);

  const spitfire::math::cVec3 lightPosition(5.0f, 5.0f, 10.0f);
  const spitfire::math::cColour lightAmbientColour(0.2, 0.2, 0.2);
  const spitfire::math::cColour lightDiffuseColour(1.0, 1.0, 1.0);
  const spitfire::math::cColour lightSpecularColour(1.0, 1.0, 1.0);

  const spitfire::math::cColour materialAmbientColour(1.0, 0.0, 1.0);
  const float fMaterialShininess = 20.0;

  const spitfire::math::cColour red(0.8f, 0.1f, 0.0f);
  const spitfire::math::cColour green(0.0f, 0.8f, 0.2f);
  const spitfire::math::cColour blue(0.2f, 0.2f, 1.0f);

  float view_rotx = 20.0f;
  float view_roty = 30.0f;
  float view_rotz = 0.0f;
  float angle = 0.0f;

  uint32_t T0 = 0;
  uint32_t Frames = 0;

  uint32_t currentTime = 0;

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();


    // Update state
    // NOTE: One difference from glxgears, we rotate based on the time so that the animation is speed independent (This should not affect the fps at all)
    currentTime = SDL_GetTicks();
    angle = currentTime / 10.0f;


    const spitfire::math::cColour clearColour(spitfire::math::random(1.0f), spitfire::math::random(1.0f), spitfire::math::random(1.0f));
    pContext->SetClearColour(clearColour);


    // Render the scene
    pContext->BeginRendering();

    const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

    spitfire::math::cMat4 matGlobalTranslation;
    matGlobalTranslation.SetTranslation(0.0f, 0.0f, -20.0f);

    spitfire::math::cMat4 matGlobalRotationX;
    matGlobalRotationX.SetRotationX(spitfire::math::DegreesToRadians(view_rotx));

    spitfire::math::cMat4 matGlobalRotationY;
    matGlobalRotationY.SetRotationY(spitfire::math::DegreesToRadians(view_roty));

    spitfire::math::cMat4 matGlobalRotationZ;
    matGlobalRotationZ.SetRotationZ(spitfire::math::DegreesToRadians(view_rotz));


    spitfire::math::cMat4 matBase = matGlobalTranslation * matGlobalRotationX * matGlobalRotationY * matGlobalRotationZ;

    {
      spitfire::math::cMat4 matLocalTranslation;
      matLocalTranslation.SetTranslation(-3.0f, -2.0f, 0.0f);

      spitfire::math::cMat4 matLocalRotationZ;
      matLocalRotationZ.SetRotationZ(spitfire::math::DegreesToRadians(angle));

      pContext->BindShader(*pShader);

      pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matBase * matLocalTranslation * matLocalRotationZ);

      // Setup ambient colour
      pContext->SetShaderConstant("ambientColour", ambientColour);

      // Setup lighting
      pContext->SetShaderConstant("lightPosition", lightPosition);
      pContext->SetShaderConstant("lightAmbientColour", lightAmbientColour);
      pContext->SetShaderConstant("lightDiffuseColour", lightDiffuseColour);
      pContext->SetShaderConstant("lightSpecularColour", lightSpecularColour);

      // Setup materials
      pContext->SetShaderConstant("materialAmbientColour", materialAmbientColour);
      pContext->SetShaderConstant("materialDiffuseColour", red);
      pContext->SetShaderConstant("materialSpecularColour", 0.5f * red);
      pContext->SetShaderConstant("fMaterialShininess", fMaterialShininess);

      pContext->BindStaticVertexBufferObject(*pGearVBO1);

      pContext->DrawStaticVertexBufferObjectTriangles(*pGearVBO1);

      pContext->UnBindStaticVertexBufferObject(*pGearVBO1);

      pContext->UnBindShader(*pShader);
    }

    {
      spitfire::math::cMat4 matLocalTranslation;
      matLocalTranslation.SetTranslation(3.1f, -2.0f, 0.0f);

      spitfire::math::cMat4 matLocalRotationZ;
      matLocalRotationZ.SetRotationZ(spitfire::math::DegreesToRadians(-2.0f * angle - 9.0f));

      pContext->BindShader(*pShader);

      pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matBase * matLocalTranslation * matLocalRotationZ);

      // Setup ambient colour
      pContext->SetShaderConstant("ambientColour", ambientColour);

      // Setup lighting
      pContext->SetShaderConstant("lightPosition", lightPosition);
      pContext->SetShaderConstant("lightAmbientColour", lightAmbientColour);
      pContext->SetShaderConstant("lightDiffuseColour", lightDiffuseColour);
      pContext->SetShaderConstant("lightSpecularColour", lightSpecularColour);

      // Setup materials
      pContext->SetShaderConstant("materialAmbientColour", materialAmbientColour);
      pContext->SetShaderConstant("materialDiffuseColour", green);
      pContext->SetShaderConstant("materialSpecularColour", 0.5f * green);
      pContext->SetShaderConstant("fMaterialShininess", fMaterialShininess);

      pContext->BindStaticVertexBufferObject(*pGearVBO2);

      pContext->DrawStaticVertexBufferObjectTriangles(*pGearVBO2);

      pContext->UnBindStaticVertexBufferObject(*pGearVBO2);

      pContext->UnBindShader(*pShader);
    }

    {
      spitfire::math::cMat4 matLocalTranslation;
      matLocalTranslation.SetTranslation(-3.1f, 4.2f, 0.0f);

      spitfire::math::cMat4 matLocalRotationZ;
      matLocalRotationZ.SetRotationZ(spitfire::math::DegreesToRadians(-2.0f * angle - 25.0f));

      pContext->BindShader(*pShader);

      pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matBase * matLocalTranslation * matLocalRotationZ);

      // Setup ambient colour
      pContext->SetShaderConstant("ambientColour", ambientColour);

      // Setup lighting
      pContext->SetShaderConstant("lightPosition", lightPosition);
      pContext->SetShaderConstant("lightAmbientColour", lightAmbientColour);
      pContext->SetShaderConstant("lightDiffuseColour", lightDiffuseColour);
      pContext->SetShaderConstant("lightSpecularColour", lightSpecularColour);

      // Setup materials
      pContext->SetShaderConstant("materialAmbientColour", materialAmbientColour);
      pContext->SetShaderConstant("materialDiffuseColour", blue);
      pContext->SetShaderConstant("materialSpecularColour", 0.5f * blue);
      pContext->SetShaderConstant("fMaterialShininess", fMaterialShininess);

      pContext->BindStaticVertexBufferObject(*pGearVBO3);

      pContext->DrawStaticVertexBufferObjectTriangles(*pGearVBO3);

      pContext->UnBindStaticVertexBufferObject(*pGearVBO3);

      pContext->UnBindShader(*pShader);
    }

    pContext->EndRendering();

    // Calculate our frames per second
    Frames++;
    {
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 1000) {
          float fps = Frames;
          printf("%g FPS\n", fps);
          T0 = t;
          Frames = 0;
      }
    }

    //.. yield
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

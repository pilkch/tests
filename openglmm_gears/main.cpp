#include <cassert>
#include <cmath>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>
#include <list>

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/util/log.h>

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

  bool Create();
  void Destroy();

  void Run();

private:
  void CreateGear(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float fInnerRadius, float fOuterRadius, float fWidth, size_t nTeeth, float fToothDepth);
  void CreateSphere(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float fRadius, size_t nSegments);

  bool LoadResources();
  void DestroyResources();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  std::vector<std::string> GetInputDescription() const;

  bool bIsDone;

  opengl::cSystem system;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  opengl::cShader* pShader;

  opengl::cStaticVertexBufferObject gearVBO1;
  opengl::cStaticVertexBufferObject gearVBO2;
  opengl::cStaticVertexBufferObject gearVBO3;
};

cApplication::cApplication() :
  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  pShader(nullptr)
{
}

bool cApplication::Create()
{
  LOG("");

  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  opengl::cResolution resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 300) || (resolution.height < 300) || ((resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8) && (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8))) {
    LOGERROR("Current screen resolution is not adequate ", resolution.width, "x", resolution.height);
    return false;
  }

  // Set our required resolution
  resolution.width = 300;
  resolution.height = 300;
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  pWindow = system.CreateWindow(TEXT("openglmm_gears"), resolution, false);
  if (pWindow == nullptr) {
    LOGERROR("Window could not be created");
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    LOGERROR("Context could not be created");
    return false;
  }

  if (!LoadResources()) {
    LOGERROR("Resources could not be loaded");
    return false;
  }

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  DestroyResources();

  pContext = nullptr;

  if (pWindow != nullptr) {
    system.DestroyWindow(pWindow);
    pWindow = nullptr;
  }
}

void cApplication::_OnWindowEvent(const opengl::cWindowEvent& event)
{
  LOG("");

  // On Windows the driver is liable to invalidate the resources when the window is resized, so we need to handle destroying and reloading them ourselves
  if (event.IsAboutToResize()) {
    #ifdef __WIN__
    DestroyResources();
    #endif
  } else if (event.IsResized()) {
    #ifdef __WIN__
    LoadResources();
    #endif
  } else if (event.IsQuit()) {
    LOG("Quiting");
    bIsDone = true;
  }
}

void cApplication::_OnMouseEvent(const opengl::cMouseEvent& event)
{
  (void)event;

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

std::vector<std::string> cApplication::GetInputDescription() const
{
  std::vector<std::string> description;
  description.push_back("Esc quit");

  return description;
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

void cApplication::CreateGear(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float fInnerRadius, float fOuterRadius, float fWidth, size_t nTeeth, float fToothDepth)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder builder;
  builder.CreateGear(fInnerRadius, fOuterRadius, fWidth, nTeeth, fToothDepth, *pGeometryDataPtr);

  staticVertexBufferObject.SetData(pGeometryDataPtr);

  staticVertexBufferObject.Compile();
}


//
// Build a sphere vbo object for rendering later.
//

void cApplication::CreateSphere(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float fRadius, size_t nSegments)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const size_t nTextureUnits = 0;

  opengl::cGeometryBuilder builder;
  builder.CreateSphere(fRadius, nSegments, *pGeometryDataPtr, nTextureUnits);

  staticVertexBufferObject.SetData(pGeometryDataPtr);

  staticVertexBufferObject.Compile();
}

bool cApplication::LoadResources()
{
  pShader = pContext->CreateShader(TEXT("shaders/gear.vert"), TEXT("shaders/gear.frag"));
  if (pShader == nullptr) {
    LOGERROR("cApplication::LoadResources Shader could not be created");
    return false;
  }

  pContext->CreateStaticVertexBufferObject(gearVBO1);
  pContext->CreateStaticVertexBufferObject(gearVBO2);
  pContext->CreateStaticVertexBufferObject(gearVBO3);

  // Make the gears
  CreateGear(gearVBO1, 1.0f, 4.0f, 1.0f, 20, 0.7f);
  CreateGear(gearVBO2, 0.5f, 2.0f, 2.0f, 10, 0.7f);
  CreateGear(gearVBO3, 1.3f, 2.0f, 0.5f, 10, 0.7f);
  //CreateSphere(gearVBO3, 1.3f, 32);

  
  // Light
  const spitfire::math::cVec3 lightPosition(5.0f, 5.0f, 10.0f);
  const spitfire::math::cColour lightAmbientColour(0.2f, 0.2f, 0.2f);
  const spitfire::math::cColour lightDiffuseColour(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour lightSpecularColour(1.0f, 1.0f, 1.0f);

  // Material
  const spitfire::math::cColour materialAmbientColour(0.0f, 0.0f, 0.0f);
  const spitfire::math::cColour materialSpecularColour(1.0f, 1.0f, 1.0f);
  const float fMaterialShininess = 50.0f;
  
  LOG("Setting shader constants ", opengl::cSystem::GetErrorString());
  // Set our shader constants
  pContext->BindShader(*pShader);

    // Setup lighting
    pContext->SetShaderConstant("light.position", lightPosition);
    pContext->SetShaderConstant("light.ambientColour", lightAmbientColour);
    pContext->SetShaderConstant("light.diffuseColour", lightDiffuseColour);
    pContext->SetShaderConstant("light.specularColour", lightSpecularColour);

    // Setup materials
    pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
    pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
    pContext->SetShaderConstant("material.fShininess", fMaterialShininess);

  pContext->UnBindShader(*pShader);

  return true;
}

void cApplication::DestroyResources()
{
  ASSERT(pContext != nullptr);

  pContext->DestroyStaticVertexBufferObject(gearVBO3);
  pContext->DestroyStaticVertexBufferObject(gearVBO2);
  pContext->DestroyStaticVertexBufferObject(gearVBO1);

  if (pShader != nullptr) {
    pContext->DestroyShader(pShader);
    pShader = nullptr;
  }
}

void cApplication::Run()
{
  LOG(opengl::cSystem::GetErrorString());

  assert(pContext != nullptr);
  assert(pContext->IsValid());

  // Print the input instructions
  const std::vector<std::string> inputDescription = GetInputDescription();
  const size_t n = inputDescription.size();
  for (size_t i = 0; i < n; i++) std::cout<<inputDescription[i]<<std::endl;

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
  
  LOG("Entering main loop ", opengl::cSystem::GetErrorString());
  while (!bIsDone) {
    // Update window events
    pWindow->ProcessEvents();


    // Update state
    // NOTE: One difference from glxgears, we rotate based on the time so that the animation is speed independent (This should not affect the fps at all)
    currentTime = SDL_GetTicks();
    angle = currentTime / 10.0f;


    const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
    pContext->SetClearColour(clearColour);


    // Render the scene
    pContext->BeginRenderToScreen();

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

      // Setup material
      pContext->SetShaderConstant("material.diffuseColour", red);

      pContext->BindStaticVertexBufferObject(gearVBO1);

      pContext->DrawStaticVertexBufferObjectTriangles(gearVBO1);

      pContext->UnBindStaticVertexBufferObject(gearVBO1);

      pContext->UnBindShader(*pShader);
    }

    {
      spitfire::math::cMat4 matLocalTranslation;
      matLocalTranslation.SetTranslation(3.1f, -2.0f, 0.0f);

      spitfire::math::cMat4 matLocalRotationZ;
      matLocalRotationZ.SetRotationZ(spitfire::math::DegreesToRadians(-2.0f * angle - 9.0f));

      pContext->BindShader(*pShader);

      pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matBase * matLocalTranslation * matLocalRotationZ);

      // Setup material
      pContext->SetShaderConstant("material.diffuseColour", green);

      pContext->BindStaticVertexBufferObject(gearVBO2);

      pContext->DrawStaticVertexBufferObjectTriangles(gearVBO2);

      pContext->UnBindStaticVertexBufferObject(gearVBO2);

      pContext->UnBindShader(*pShader);
    }

    {
      spitfire::math::cMat4 matLocalTranslation;
      matLocalTranslation.SetTranslation(-3.1f, 4.2f, 0.0f);

      spitfire::math::cMat4 matLocalRotationZ;
      matLocalRotationZ.SetRotationZ(spitfire::math::DegreesToRadians(-2.0f * angle - 25.0f));

      pContext->BindShader(*pShader);

      pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matBase * matLocalTranslation * matLocalRotationZ);

      // Setup material
      pContext->SetShaderConstant("material.diffuseColour", blue);

      pContext->BindStaticVertexBufferObject(gearVBO3);

      pContext->DrawStaticVertexBufferObjectTriangles(gearVBO3);

      pContext->UnBindStaticVertexBufferObject(gearVBO3);

      pContext->UnBindShader(*pShader);
    }

    pContext->EndRenderToScreen(*pWindow);

    // Calculate our frames per second
    Frames++;
    {
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 1000) {
          float fps = float(Frames);
          LOG(fps, " FPS");
          T0 = t;
          Frames = 0;
      }
    }

    //.. yield
  };
}

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  cApplication application;

  const bool bIsSuccess = application.Create();
  if (bIsSuccess) application.Run();

  application.Destroy();

  return bIsSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}

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

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();
  ~cApplication();

  bool Create();
  void Destroy();

  void Run();

private:
  void CreateBoxVBO();
  void CreateScreenQuadVBO();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  bool bIsRotating;
  bool bIsWireframe;
  bool bIsDone;

  opengl::cSystem system;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  opengl::cTextureFrameBufferObject* pTextureFrameBufferObject;

  opengl::cTexture* pTextureDiffuse;
  opengl::cTexture* pTextureDetail;

  opengl::cShader* pShaderCrate;
  opengl::cShader* pShaderScreenQuad;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObject;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectScreenQuad;
};

cApplication::cApplication() :
  bIsRotating(true),
  bIsWireframe(false),
  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  pTextureFrameBufferObject(nullptr),

  pTextureDiffuse(nullptr),
  pTextureDetail(nullptr),

  pShaderCrate(nullptr),
  pShaderScreenQuad(nullptr),

  pStaticVertexBufferObject(nullptr),
  pStaticVertexBufferObjectScreenQuad(nullptr)
{
}

cApplication::~cApplication()
{
  Destroy();
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

void cApplication::CreateScreenQuadVBO()
{
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float_t fHalfSize = 0.125f;
  const spitfire::math::cVec2 vMin(-fHalfSize, -fHalfSize);
  const spitfire::math::cVec2 vMax(fHalfSize, fHalfSize);

  opengl::cGeometryBuilder_v2_t2 builder(*pGeometryDataPtr);

  // Front facing quad
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMax.y), spitfire::math::cVec2(1.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(1.0f, 1.0f));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMin.y), spitfire::math::cVec2(0.0f, 1.0f));

  pStaticVertexBufferObjectScreenQuad->SetData(pGeometryDataPtr);

  pStaticVertexBufferObjectScreenQuad->Compile2D(system);
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

  pWindow = system.CreateWindow(TEXT("OpenGLmm FBO Test"), resolution, false);
  if (pWindow == nullptr) {
    std::cout<<"Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }

  pTextureFrameBufferObject = pContext->CreateTextureFrameBufferObject(512, 512, opengl::PIXELFORMAT::R8G8B8A8);
  assert(pTextureFrameBufferObject != nullptr);

  pTextureDiffuse = pContext->CreateTexture(TEXT("textures/diffuse.png"));
  assert(pTextureDiffuse != nullptr);
  pTextureDetail = pContext->CreateTexture(TEXT("textures/detail.png"));
  assert(pTextureDetail != nullptr);

  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  assert(pShaderCrate != nullptr);

  pShaderScreenQuad = pContext->CreateShader(TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(pShaderScreenQuad != nullptr);

  pStaticVertexBufferObject = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObject != nullptr);
  CreateBoxVBO();

  pStaticVertexBufferObjectScreenQuad = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);
  CreateScreenQuadVBO();

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  if (pStaticVertexBufferObjectScreenQuad != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectScreenQuad);
    pStaticVertexBufferObjectScreenQuad = nullptr;
  }

  if (pStaticVertexBufferObject != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObject);
    pStaticVertexBufferObject = nullptr;
  }

  if (pShaderScreenQuad != nullptr) {
    pContext->DestroyShader(pShaderScreenQuad);
    pShaderScreenQuad = nullptr;
  }

  if (pShaderCrate != nullptr) {
    pContext->DestroyShader(pShaderCrate);
    pShaderCrate = nullptr;
  }

  if (pTextureDetail != nullptr) {
    pContext->DestroyTexture(pTextureDetail);
    pTextureDetail = nullptr;
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
        std::cout<<"cApplication::_OnKeyboardEvent w up"<<std::endl;
        bIsWireframe = !bIsWireframe;
        break;
      }
    }
  }
}

void cApplication::Run()
{
  std::cout<<"cApplication::Run"<<std::endl;

  assert(pContext != nullptr);
  assert(pContext->IsValid());
  assert(pTextureDiffuse != nullptr);
  assert(pTextureDiffuse->IsValid());
  assert(pTextureDetail != nullptr);
  assert(pTextureDetail->IsValid());
  assert(pShaderCrate != nullptr);
  assert(pShaderCrate->IsCompiledProgram());
  assert(pShaderScreenQuad != nullptr);
  assert(pShaderScreenQuad->IsCompiledProgram());
  assert(pStaticVertexBufferObject != nullptr);
  assert(pStaticVertexBufferObject->IsCompiled());
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);
  assert(pStaticVertexBufferObjectScreenQuad->IsCompiled());

  const spitfire::math::cVec3 position(5.0f, 10.0f, 0.0f);

  spitfire::math::cMat4 matTranslation;
  matTranslation.SetTranslation(position);

  spitfire::math::cQuaternion rotation;
  spitfire::math::cMat4 matRotation;


  const spitfire::math::cVec3 axis(0.0f, 0.0f, 1.0f);
  float fAngleRadians = 0.0f;
  const float fRotationSpeed = 0.001f;

  uint32_t T0 = 0;
  uint32_t Frames = 0;

  uint32_t previousTime = SDL_GetTicks();
  uint32_t currentTime = SDL_GetTicks();

  pContext->EnableLighting();

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();


    // Update state
    previousTime = currentTime;
    currentTime = SDL_GetTicks();

    if (bIsRotating) fAngleRadians += float(currentTime - previousTime) * fRotationSpeed;



    {
      // Render the scene into the frame buffer object
      const spitfire::math::cColour clearColour(0.0f, 1.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObject);

      if (bIsWireframe) pContext->EnableWireframe();

      const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

      const spitfire::math::cVec3 eye(position + spitfire::math::cVec3(0.0f, -2.5f, 1.0f));
      const spitfire::math::cVec3 target(position);
      spitfire::math::cMat4 matModelView;
      matModelView.LookAt(eye, target, spitfire::math::v3Up);

      rotation.SetFromAxisAngle(axis, fAngleRadians);

      matRotation.SetRotation(rotation);


      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);

      pContext->BindShader(*pShaderCrate);

      pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matModelView * matTranslation * matRotation);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObject);

      pContext->UnBindShader(*pShaderCrate);

      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);

      pContext->EndRenderToTexture(*pTextureFrameBufferObject);
    }

    {
      // Render the scene with the new texture
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRendering();

      if (bIsWireframe) pContext->EnableWireframe();

      const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

      const spitfire::math::cVec3 eye(position + spitfire::math::cVec3(0.0f, -2.5f, 1.0f));
      const spitfire::math::cVec3 target(position);
      spitfire::math::cMat4 matModelView;
      matModelView.LookAt(eye, target, spitfire::math::v3Up);

      rotation.SetFromAxisAngle(axis, fAngleRadians);

      matRotation.SetRotation(rotation);


      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);

      pContext->BindShader(*pShaderCrate);

      pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matModelView * matTranslation * matRotation);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObject);

      pContext->UnBindShader(*pShaderCrate);

      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);


      // Now draw an overlay of our rendered texture
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      // Move the quad into the bottom right hand corner of the screen
      spitfire::math::cMat4 matModelView2D;
      matModelView2D.SetTranslation(0.75f + (0.5f * 0.25f), 0.75f + (0.5f * 0.25f), 0.0f);

      pContext->BindTexture(0, *pTextureFrameBufferObject);

      pContext->BindShader(*pShaderScreenQuad);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenQuad);

      {
        pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectScreenQuad);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenQuad);

      pContext->UnBindShader(*pShaderScreenQuad);

      pContext->UnBindTexture(0, *pTextureFrameBufferObject);

      pContext->EndRenderMode2D();

      pContext->EndRendering();
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


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
#include <libopenglmm/cFont.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

#ifndef BUILD_OPENGLMM_FONT
#error "BUILD_OPENGLMM_FONT has not been defined"
#endif

class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();
  ~cApplication();

  bool Create();
  void Destroy();

  void Run();

private:
  void CreateBox();
  void CreateText();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  std::vector<std::string> GetInputDescription() const;

  bool bIsRotating;
  bool bIsWireframe;
  bool bIsDone;

  opengl::cSystem system;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  // Text
  opengl::cFont* pFont;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectText;

  // Crate
  opengl::cTexture* pTextureCrate;
  opengl::cShader* pShaderCrate;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectCrate;
};

cApplication::cApplication() :
  bIsRotating(true),
  bIsWireframe(false),
  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  pFont(nullptr),

  pStaticVertexBufferObjectText(nullptr),

  pTextureCrate(nullptr),
  pShaderCrate(nullptr),
  pStaticVertexBufferObjectCrate(nullptr)
{
}

cApplication::~cApplication()
{
  Destroy();
}

void cApplication::CreateText()
{
  assert(pFont != nullptr);
  assert(pStaticVertexBufferObjectText != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v2_c4_t2 builder(*pGeometryDataPtr);

  // Front facing quad
  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);
  pFont->PushBack(builder, TEXT("1234567890 abcdefghijklmnopqrstuvwxyz"), red, spitfire::math::cVec2(0.0f, 0.3f));
  const spitfire::math::cColour green(0.0f, 1.0f, 0.0f);
  pFont->PushBack(builder, TEXT("abcdefghijklmnopqrstuvwxyz 1234567890"), green, spitfire::math::cVec2(0.0f, 0.5f));

  pStaticVertexBufferObjectText->SetData(pGeometryDataPtr);

  pStaticVertexBufferObjectText->Compile2D(system);
}

void cApplication::CreateBox()
{
  assert(pStaticVertexBufferObjectCrate != nullptr);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float_t fWidth = 0.5f;
  const float_t fDepth = 0.5f;
  const float_t fHeight = 0.5f;
  const size_t nTextureCoordinates = 1;

  opengl::cGeometryBuilder builder;

  builder.CreateBox(fWidth, fDepth, fHeight, *pGeometryDataPtr, nTextureCoordinates);

  pStaticVertexBufferObjectCrate->SetData(pGeometryDataPtr);

  pStaticVertexBufferObjectCrate->Compile(system);
}

bool cApplication::Create()
{
  std::cout<<"cApplication::Create"<<std::endl;

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

  pWindow = system.CreateWindow(TEXT("OpenGLmm Font Test"), resolution, false);
  if (pWindow == nullptr) {
    std::cout<<"Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }

  pFont = pContext->CreateFont(TEXT("fonts/pricedown.ttf"), 32, TEXT("shaders/font.vert"), TEXT("shaders/font.frag"));

  pStaticVertexBufferObjectText = pContext->CreateStaticVertexBufferObject();
  CreateText();

  pTextureCrate = pContext->CreateTexture(TEXT("textures/crate.png"));

  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));

  pStaticVertexBufferObjectCrate = pContext->CreateStaticVertexBufferObject();
  CreateBox();

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  if (pStaticVertexBufferObjectCrate != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectCrate);
    pStaticVertexBufferObjectCrate = nullptr;
  }

  if (pShaderCrate != nullptr) {
    pContext->DestroyShader(pShaderCrate);
    pShaderCrate = nullptr;
  }

  if (pTextureCrate != nullptr) {
    pContext->DestroyTexture(pTextureCrate);
    pTextureCrate = nullptr;
  }

  if (pStaticVertexBufferObjectText != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectText);
    pStaticVertexBufferObjectText = nullptr;
  }

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
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case SDLK_w: {
        std::cout<<"cApplication::_OnKeyboardEvent w key up"<<std::endl;
        bIsWireframe = !bIsWireframe;
        break;
      }
      case SDLK_SPACE: {
        std::cout<<"cApplication::_OnKeyboardEvent spacebar up"<<std::endl;
        bIsRotating = !bIsRotating;
        break;
      }
    }
  }
}

std::vector<std::string> cApplication::GetInputDescription() const
{
  std::vector<std::string> description;
  description.push_back("Space toggle rotation");
  description.push_back("W toggle wireframe");
  description.push_back("Esc quit");

  return description;
}

void cApplication::Run()
{
  assert(pContext != nullptr);
  assert(pContext->IsValid());

  // Text
  assert(pFont != nullptr);
  assert(pFont->IsValid());
  assert(pStaticVertexBufferObjectText != nullptr);
  assert(pStaticVertexBufferObjectText->IsCompiled());

  // Crate
  assert(pTextureCrate != nullptr);
  assert(pTextureCrate->IsValid());
  assert(pShaderCrate != nullptr);
  assert(pShaderCrate->IsCompiledProgram());
  assert(pStaticVertexBufferObjectCrate != nullptr);
  assert(pStaticVertexBufferObjectCrate->IsCompiled());

  // Print the input instructions
  const std::vector<std::string> inputDescription = GetInputDescription();
  const size_t n = inputDescription.size();
  for (size_t i = 0; i < n; i++) std::cout<<inputDescription[i]<<std::endl;

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

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();


    // Update state
    previousTime = currentTime;
    currentTime = SDL_GetTicks();

    if (bIsRotating) fAngleRadians += float(currentTime - previousTime) * fRotationSpeed;


    {
      // Render the scene with the new texture
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToScreen();

      if (bIsWireframe) pContext->EnableWireframe();

      // Draw the crate

      {
        const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

        const spitfire::math::cVec3 eye(position + spitfire::math::cVec3(0.0f, -2.5f, 1.0f));
        const spitfire::math::cVec3 target(position);
        spitfire::math::cMat4 matModelView;
        matModelView.LookAt(eye, target, spitfire::math::v3Up);

        rotation.SetFromAxisAngle(axis, fAngleRadians);

        matRotation.SetRotation(rotation);

        pContext->BindTexture(0, *pTextureCrate);

        pContext->BindShader(*pShaderCrate);

        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectCrate);

        {
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matModelView * matTranslation * matRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObjectCrate);
        }

        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectCrate);

        pContext->UnBindShader(*pShaderCrate);

        pContext->UnBindTexture(0, *pTextureCrate);
      }


      // Draw the text overlay
      {
        pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

        pContext->BindFont(*pFont);

        // Rendering the font in the middle of the screen
        spitfire::math::cMat4 matModelView;
        matModelView.SetTranslation(0.1f, 0.1f, 0.0f);

        pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView);

        pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

        {
          pContext->DrawStaticVertexBufferObjectTriangles2D(*pStaticVertexBufferObjectText);
        }

        pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectText);

        pContext->UnBindFont(*pFont);

        pContext->EndRenderMode2D();
      }

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


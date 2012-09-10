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
  void CreateScreenRectVBO();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  std::vector<std::string> GetInputDescription() const;

  bool bIsRotating;
  bool bIsWireframe;
  bool bIsDone;

  // For controlling the rotation of the trackball
  spitfire::math::cQuaternion rotationX;
  spitfire::math::cQuaternion rotationZ;

  opengl::cSystem system;

  opengl::cResolution resolution;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  opengl::cTextureFrameBufferObject* pTextureFrameBufferObject;

  opengl::cTexture* pTextureDiffuse;
  opengl::cTexture* pTextureDetail;

  opengl::cShader* pShaderCrate;
  opengl::cShader* pShaderScreenRect;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObject;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectScreenRect;
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
  pShaderScreenRect(nullptr),

  pStaticVertexBufferObject(nullptr),
  pStaticVertexBufferObjectScreenRect(nullptr)
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
  pTextureDetail = pContext->CreateTexture(TEXT("textures/detail.png"));
  assert(pTextureDetail != nullptr);

  pShaderCrate = pContext->CreateShader(TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  assert(pShaderCrate != nullptr);

  pShaderScreenRect = pContext->CreateShader(TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(pShaderScreenRect != nullptr);

  pStaticVertexBufferObject = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObject != nullptr);
  CreateBoxVBO();

  pStaticVertexBufferObjectScreenRect = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectScreenRect != nullptr);
  CreateScreenRectVBO();

  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
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

  if (event.IsMouseMove()) {
    std::cout<<"cApplication::_OnMouseEvent Mouse move"<<std::endl;

    if (fabs(event.GetX() - (pWindow->GetWidth() * 0.5f)) > 0.5f) {
      const spitfire::math::cVec3 axisIncrementZ(0.0f, 0.0f, 1.0f);
      spitfire::math::cQuaternion rotationIncrementZ;
      rotationIncrementZ.SetFromAxisAngle(axisIncrementZ, -0.001f * (event.GetX() - (pWindow->GetWidth() * 0.5f)));
      rotationZ *= rotationIncrementZ;
    }

    if (fabs(event.GetY() - (pWindow->GetHeight() * 0.5f)) > 1.5f) {
      const spitfire::math::cVec3 axisIncrementX(1.0f, 0.0f, 0.0f);
      spitfire::math::cQuaternion rotationIncrementX;
      rotationIncrementX.SetFromAxisAngle(axisIncrementX, -0.0005f * (event.GetY() - (pWindow->GetHeight() * 0.5f)));
      rotationX *= rotationIncrementX;
    }

    spitfire::math::cVec3 newAxis = rotationZ.GetAxis();
    //float fNewAngleDegrees = rotationZ.GetAngleDegrees();
    //std::cout<<"cApplication::_OnMouseEvent z newAxis={ "<<newAxis.x<<", "<<newAxis.y<<", "<<newAxis.z<<" } angle="<<fNewAngleDegrees<<std::endl;

    newAxis = rotationX.GetAxis();
    //fNewAngleDegrees = rotationX.GetAngleDegrees();
    //std::cout<<"cApplication::_OnMouseEvent x newAxis={ "<<newAxis.x<<", "<<newAxis.y<<", "<<newAxis.z<<" } angle="<<fNewAngleDegrees<<std::endl;
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

std::vector<std::string> cApplication::GetInputDescription() const
{
  std::vector<std::string> description;
  description.push_back("Space pause rotation");
  description.push_back("W toggle wireframe");
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
  assert(pTextureDetail != nullptr);
  assert(pTextureDetail->IsValid());
  assert(pShaderCrate != nullptr);
  assert(pShaderCrate->IsCompiledProgram());
  assert(pShaderScreenRect != nullptr);
  assert(pShaderScreenRect->IsCompiledProgram());
  assert(pStaticVertexBufferObject != nullptr);
  assert(pStaticVertexBufferObject->IsCompiled());
  assert(pStaticVertexBufferObjectScreenRect != nullptr);
  assert(pStaticVertexBufferObjectScreenRect->IsCompiled());

  // Print the input instructions
  const std::vector<std::string> inputDescription = GetInputDescription();
  const size_t n = inputDescription.size();
  for (size_t i = 0; i < n; i++) std::cout<<inputDescription[i]<<std::endl;

  // Center the camera at the middle of the objects
  spitfire::math::cMat4 matTranslation;
  matTranslation.SetTranslation(0.0f, 0.0f, 0.0f);

  // Set the defaults for the orbiting camera
  {
    const spitfire::math::cVec3 axisZ(0.0f, 0.0f, 1.0f);
    rotationZ.SetFromAxisAngleDegrees(axisZ, 0.0f);

    const spitfire::math::cVec3 axisX(1.0f, 0.0f, 0.0f);
    rotationX.SetFromAxisAngleDegrees(axisX, -20.0f);
  }
  const float fZoom = 10.0f;

  spitfire::math::cMat4 matRotation;


  uint32_t T0 = 0;
  uint32_t Frames = 0;

  //uint32_t previousTime = SDL_GetTicks();
  //uint32_t currentTime = SDL_GetTicks();

  pContext->EnableLighting();

  // Setup mouse
  pWindow->ShowCursor(false);
  pWindow->WarpCursorToMiddleOfScreen();

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();

    // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
    pWindow->WarpCursorToMiddleOfScreen();

    // Update state
    //previousTime = currentTime;
    //currentTime = SDL_GetTicks();

    matRotation.SetRotation(rotationZ * rotationX);

    const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

    const spitfire::math::cVec3 offset = matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, -fZoom, 0.0f));
    const spitfire::math::cVec3 up = matRotation.GetRotatedVec3(spitfire::math::v3Up);

    const spitfire::math::cVec3 target(0.0f, 0.0f, 0.0f);
    const spitfire::math::cVec3 eye(target + offset);
    spitfire::math::cMat4 matModelView;
    matModelView.LookAt(eye, target, up);


    {
      // Render the scene into the frame buffer object
      const spitfire::math::cColour clearColour(0.0f, 1.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObject);

      if (bIsWireframe) pContext->EnableWireframe();

      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);

      pContext->BindShader(*pShaderCrate);

      pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matModelView * matTranslation);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObject);

      pContext->UnBindShader(*pShaderCrate);

      pContext->UnBindTexture(1, *pTextureDetail);
      pContext->UnBindTexture(0, *pTextureDiffuse);

      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToTexture(*pTextureFrameBufferObject);
    }

    {
      // Render the scene with the new texture
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRendering();

      if (bIsWireframe) pContext->EnableWireframe();

      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureDetail);

      pContext->BindShader(*pShaderCrate);

      pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObject);

      {
        pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matModelView * matTranslation);

        pContext->DrawStaticVertexBufferObjectTriangles(*pStaticVertexBufferObject);
      }

      pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObject);

      pContext->UnBindShader(*pShaderCrate);

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


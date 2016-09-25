// Application headers
#include "main.h"
#include "tronglow.h"

// ** cTronGlow

void cTronGlow::Init(cApplication& application, opengl::cContext& context)
{
  context.CreateShader(shaderBlack, TEXT("shaders/black.vert"), TEXT("shaders/black.frag"));
  assert(shaderBlack.IsCompiledProgram());
  context.CreateShader(shaderGlowHighlights, TEXT("shaders/glowhighlights.vert"), TEXT("shaders/glowhighlights.frag"));
  assert(shaderGlowHighlights.IsCompiledProgram());
  context.CreateShader(shaderTronGlowScreen, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/tronglow.frag"));
  assert(shaderTronGlowScreen.IsCompiledProgram());
}

void cTronGlow::Destroy(opengl::cContext& context)
{
  context.DestroyShader(shaderBlack);
  context.DestroyShader(shaderGlowHighlights);
  context.DestroyShader(shaderTronGlowScreen);
}

void cTronGlow::Resize(cApplication& application, opengl::cContext& context)
{
}

void cTronGlow::AddNonGlowingObject(const spitfire::math::cMat4& matModel, opengl::cStaticVertexBufferObject* pVBO)
{
  assert(pVBO != nullptr);
  assert(pVBO->IsCompiled());

  lNotGlowingObjects.push_back(std::make_pair(matModel, pVBO));
}

void cTronGlow::BeginRender(cApplication& application, opengl::cContext& context, const spitfire::math::cMat4& matProjection, const spitfire::math::cMat4& matView, opengl::cTextureFrameBufferObject& temp)
{
  // Render the black and glowing pixels to a temporary texture
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(temp);

  // Render black objects
  context.BindShader(shaderBlack);

  for (const auto& object : lNotGlowingObjects) {
    context.BindStaticVertexBufferObject(*(object.second));
    context.SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, object.first);
    context.DrawStaticVertexBufferObjectTriangles(*(object.second));
    context.UnBindStaticVertexBufferObject(*(object.second));
  }

  context.UnBindShader(shaderBlack);

  context.BindShader(shaderGlowHighlights);

  // Now hand back control to the caller to render the glowing objects
}

void cTronGlow::RenderSceneWithTronGlow(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& temp, opengl::cTextureFrameBufferObject& output)
{
  // Render the HDRColorBuffer to our framebuffer
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(output);

  context.BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

  context.BindShader(shaderTronGlowScreen);
  context.BindTexture(0, input);
  context.BindTexture(1, temp);
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
  context.UnBindTexture(1, temp);
  context.UnBindTexture(0, input);
  context.UnBindShader(shaderTronGlowScreen);

  context.EndRenderMode2D();

  context.EndRenderToTexture(output);
}

void cTronGlow::EndRender(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& temp, opengl::cTextureFrameBufferObject& output)
{
  context.UnBindShader(shaderGlowHighlights);

  context.EndRenderToTexture(temp);

  // Now render the blur textures

  // Combine the blur textures

  // Now render the normal texture mixed with the blur texture to the screen
  RenderSceneWithTronGlow(application, context, input, temp, output);


  // Clear the list of black objects
  lNotGlowingObjects.clear();
}

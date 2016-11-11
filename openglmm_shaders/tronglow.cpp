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

  context.CreateTextureFrameBufferObjectWithDepth(fboGlow, context.GetWidth() / 4, context.GetHeight() / 4);
  ASSERT(fboGlow.IsValid());
  fboGlow.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboGlow.SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);

  blur.Init(application, context, context.GetWidth(), context.GetHeight());
}

void cTronGlow::Destroy(opengl::cContext& context)
{
  blur.Destroy(context);

  if (fboGlow.IsValid()) context.DestroyTextureFrameBufferObject(fboGlow);

  context.DestroyShader(shaderBlack);
  context.DestroyShader(shaderGlowHighlights);
  context.DestroyShader(shaderTronGlowScreen);
}

void cTronGlow::Resize(cApplication& application, opengl::cContext& context)
{
  blur.Resize(context);
}

void cTronGlow::ReloadShaders(opengl::cContext& context)
{
  blur.ReloadShaders(context);

  if (shaderBlack.IsCompiledProgram()) context.DestroyShader(shaderBlack);
  if (shaderGlowHighlights.IsCompiledProgram()) context.DestroyShader(shaderGlowHighlights);
  if (shaderTronGlowScreen.IsCompiledProgram()) context.DestroyShader(shaderTronGlowScreen);

  context.CreateShader(shaderBlack, TEXT("shaders/black.vert"), TEXT("shaders/black.frag"));
  assert(shaderBlack.IsCompiledProgram());
  context.CreateShader(shaderGlowHighlights, TEXT("shaders/glowhighlights.vert"), TEXT("shaders/glowhighlights.frag"));
  assert(shaderGlowHighlights.IsCompiledProgram());
  context.CreateShader(shaderTronGlowScreen, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/tronglow.frag"));
  assert(shaderTronGlowScreen.IsCompiledProgram());
}

void cTronGlow::AddNonGlowingObject(const spitfire::math::cMat4& matModel, opengl::cStaticVertexBufferObject* pVBO)
{
  assert(pVBO != nullptr);
  assert(pVBO->IsCompiled());

  lNotGlowingObjects.push_back(std::make_pair(matModel, pVBO));
}

void cTronGlow::BeginRender(cApplication& application, opengl::cContext& context, const spitfire::math::cMat4& matProjection, const spitfire::math::cMat4& matView, opengl::cTextureFrameBufferObject& temp0)
{
  // Render the black and glowing pixels to a temporary texture
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(temp0);

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

void cTronGlow::RenderSceneWithTronGlow(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& glow, opengl::cTextureFrameBufferObject& brightPixels, opengl::cTextureFrameBufferObject& output)
{
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(output);

  context.BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

  context.BindShader(shaderTronGlowScreen);

  context.SetShaderConstant("iGlowTextureSize", 4);

  context.BindTexture(0, input);
  context.BindTexture(1, glow);
  context.BindTexture(2, brightPixels);
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
  context.UnBindTexture(2, brightPixels);
  context.UnBindTexture(1, glow);
  context.UnBindTexture(0, input);
  context.UnBindShader(shaderTronGlowScreen);

  context.EndRenderMode2D();

  context.EndRenderToTexture(output);
}

void cTronGlow::EndRender(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& temp0, opengl::cTextureFrameBufferObject& temp1, opengl::cTextureFrameBufferObject& output)
{
  context.UnBindShader(shaderGlowHighlights);

  context.EndRenderToTexture(temp0);

  // Create blur texture
  const size_t uiBlurRadius = 48;
  blur.Render(application, context, temp0, temp1, fboGlow, uiBlurRadius);

  // Combine the blur textures

  // Now render the normal texture mixed with the blur texture to the screen
  RenderSceneWithTronGlow(application, context, input, fboGlow, temp0, output);


  // Clear the list of black objects
  lNotGlowingObjects.clear();
}

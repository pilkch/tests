// Application headers
#include "main.h"
#include "heathaze.h"

// ** cHeatHaze

void cHeatHaze::Init(cApplication& application, opengl::cContext& context)
{
  context.CreateShader(shaderBlack, TEXT("shaders/black.vert"), TEXT("shaders/black.frag"));
  assert(shaderBlack.IsCompiledProgram());
  context.CreateShader(shaderHeatHighlights, TEXT("shaders/heathazevolumes.vert"), TEXT("shaders/heathazevolumes.frag"));
  assert(shaderHeatHighlights.IsCompiledProgram());
  context.CreateShader(shaderHeatHazeScreen, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/heathaze.frag"));
  assert(shaderHeatHazeScreen.IsCompiledProgram());

  context.CreateTexture(textureNoiseTiled, TEXT("textures/noise_tiled.png"));
  ASSERT(textureNoiseTiled.IsValid());
  textureNoiseTiled.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  textureNoiseTiled.SetWrap(opengl::TEXTURE_WRAP::REPEAT);

  context.CreateTextureFrameBufferObjectWithDepth(fboHeatMap, context.GetWidth(), context.GetHeight());
  ASSERT(fboHeatMap.IsValid());
  fboHeatMap.SetMagFilter(opengl::TEXTURE_FILTER::LINEAR);
  fboHeatMap.SetWrap(opengl::TEXTURE_WRAP::CLAMP_TO_EDGE);
}

void cHeatHaze::Destroy(opengl::cContext& context)
{
  if (fboHeatMap.IsValid()) context.DestroyTextureFrameBufferObject(fboHeatMap);

  if (textureNoiseTiled.IsValid()) context.DestroyTexture(textureNoiseTiled);

  context.DestroyShader(shaderBlack);
  context.DestroyShader(shaderHeatHighlights);
  context.DestroyShader(shaderHeatHazeScreen);
}

void cHeatHaze::ReloadShaders(opengl::cContext& context)
{
  if (shaderBlack.IsCompiledProgram()) context.DestroyShader(shaderBlack);
  if (shaderHeatHighlights.IsCompiledProgram()) context.DestroyShader(shaderHeatHighlights);
  if (shaderHeatHazeScreen.IsCompiledProgram()) context.DestroyShader(shaderHeatHazeScreen);

  context.CreateShader(shaderBlack, TEXT("shaders/black.vert"), TEXT("shaders/black.frag"));
  assert(shaderBlack.IsCompiledProgram());
  context.CreateShader(shaderHeatHighlights, TEXT("shaders/heathazevolumes.vert"), TEXT("shaders/heathazevolumes.frag"));
  assert(shaderHeatHighlights.IsCompiledProgram());
  context.CreateShader(shaderHeatHazeScreen, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/heathaze.frag"));
  assert(shaderHeatHazeScreen.IsCompiledProgram());
}

void cHeatHaze::AddColdObject(const spitfire::math::cMat4& matModel, opengl::cStaticVertexBufferObject* pVBO)
{
  assert(pVBO != nullptr);
  assert(pVBO->IsCompiled());

  lColdObjects.push_back(std::make_pair(matModel, pVBO));
}

void cHeatHaze::BeginRender(cApplication& application, opengl::cContext& context, const spitfire::math::cMat4& matProjection, const spitfire::math::cMat4& matView)
{
  // Render the black and glowing pixels to a heat map texture
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(fboHeatMap);

  // Render black objects
  context.BindShader(shaderBlack);

  for (const auto& object : lColdObjects) {
    context.BindStaticVertexBufferObject(*(object.second));
    context.SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, object.first);
    context.DrawStaticVertexBufferObjectTriangles(*(object.second));
    context.UnBindStaticVertexBufferObject(*(object.second));
  }

  context.UnBindShader(shaderBlack);

  context.BindShader(shaderHeatHighlights);

  // Now hand back control to the caller to render the glowing objects
}

void cHeatHaze::EndRender(cApplication& application, opengl::cContext& context, spitfire::durationms_t time, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& output)
{
  context.UnBindShader(shaderHeatHighlights);

  context.EndRenderToTexture(fboHeatMap);

  // Now render the normal texture mixed with the heat texture to the screen
  const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
  context.SetClearColour(clearColour);

  context.BeginRenderToTexture(output);

  context.BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

  context.BindShader(shaderHeatHazeScreen);

  context.SetShaderConstant("screenSize", spitfire::math::cVec2(context.GetResolution().width, context.GetResolution().height));
  context.SetShaderConstant("currentTime", float(time));

  context.BindTexture(0, input);
  context.BindTexture(1, fboHeatMap);
  context.BindTexture(2, textureNoiseTiled);
  application.RenderScreenRectangleShaderAndTextureAlreadySet();
  context.UnBindTexture(2, textureNoiseTiled);
  context.UnBindTexture(1, fboHeatMap);
  context.UnBindTexture(0, input);
  context.UnBindShader(shaderHeatHazeScreen);

  context.EndRenderMode2D();

  context.EndRenderToTexture(output);


  // Clear the list of black objects
  lColdObjects.clear();
}

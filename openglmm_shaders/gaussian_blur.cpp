// Standard headers
#include <cassert>

// OpenGL headers
#include <GL/GLee.h>
#include <GL/glu.h>

// Spitfire headers
#include <spitfire/spitfire.h>

// Application headers
#include "gaussian_blur.h"
#include "main.h"

// ** cGaussianBlur

void cGaussianBlur::Init(cApplication& application, opengl::cContext& context, size_t fboWidth, size_t fboHeight)
{
  context.CreateShader(shaderGaussBlur, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/gaussian_blur.frag"));
  ASSERT(shaderGaussBlur.IsCompiledProgram());

  application.CreateScreenRectVBO(vbo, 1.0f, 1.0f, fboWidth, fboHeight);
}

void cGaussianBlur::Destroy(opengl::cContext& context)
{
  if (shaderGaussBlur.IsCompiledProgram()) context.DestroyShader(shaderGaussBlur);

  context.DestroyStaticVertexBufferObject(vbo);
}

void cGaussianBlur::Resize(opengl::cContext& context)
{
}

void cGaussianBlur::Render(cApplication& application, opengl::cContext& context, opengl::cTextureFrameBufferObject& input, opengl::cTextureFrameBufferObject& temp, opengl::cTextureFrameBufferObject& output, unsigned int uiRadiusPixels)
{
  context.BindShader(shaderGaussBlur);
  context.SetShaderConstant("uBlurRadius", int(uiRadiusPixels));

  // Horizontal pass
  context.BeginRenderToTexture(temp);
  context.SetShaderConstant("uBlurDirection", spitfire::math::cVec2(1.0f, 0.0f));
  context.BindTexture(0, input);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vbo);
  context.UnBindTexture(0, input);
  context.EndRenderToTexture(temp);

  // Vertical pass
  context.BeginRenderToTexture(output);
  context.SetShaderConstant("uBlurDirection", spitfire::math::cVec2(0.0f, 1.0f));
  context.BindTexture(0, temp);
  application.RenderScreenRectangleShaderAndTextureAlreadySet(vbo);
  context.UnBindTexture(0, temp);
  context.EndRenderToTexture(output);

  context.UnBindShader(shaderGaussBlur);
}

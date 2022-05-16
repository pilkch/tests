#include <cassert>
#include <cmath>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>
#include <list>

#ifdef __WIN__
// OpenGL headers
#include <GL/GLee.h>
#include <GL/glu.h>
#endif

// SDL headers
#include <SDL2/SDL_image.h>

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/math/math.h>
#include <spitfire/math/cVec2.h>
#include <spitfire/math/cVec3.h>
#include <spitfire/math/cVec4.h>
#include <spitfire/math/cMat4.h>
#include <spitfire/math/cQuaternion.h>
#include <spitfire/math/cColour.h>

#include <spitfire/storage/file.h>
#include <spitfire/storage/filesystem.h>
#include <spitfire/util/log.h>

// libopenglmm headers
#include <libopenglmm/libopenglmm.h>
#include <libopenglmm/cContext.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>

// Application headers
#include "pbr.h"
#include "main.h"

namespace {

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
/*void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            // positions        // texture Coords // normal
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}*/

#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

void renderCube()
{
  // initialize (if necessary)
  if (cubeVAO == 0) {
		float vertices[] = {
      // positions          // normal    // texture Coords
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);

    unsigned int shaderAttribute = 0;
    const size_t nStrideBytes = 8 * sizeof(float);

    // Position
		glEnableVertexAttribArray(shaderAttribute);
		glVertexAttribPointer(shaderAttribute, 3, GL_FLOAT, GL_FALSE, nStrideBytes, (void*)0);
    shaderAttribute++;

    // Normal
		glEnableVertexAttribArray(shaderAttribute);
		glVertexAttribPointer(shaderAttribute, 3, GL_FLOAT, GL_FALSE, nStrideBytes, (void*)(3 * sizeof(float)));
    shaderAttribute++;

    // Texture coordinates
		glEnableVertexAttribArray(shaderAttribute);
		glVertexAttribPointer(shaderAttribute, 2, GL_FLOAT, GL_FALSE, nStrideBytes, (void*)(6 * sizeof(float)));
    shaderAttribute++;

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

    assert(cubeVAO != 0);
  }

  // render Cube
  glBindVertexArray(cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO = 0;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

#if 0
void renderCubeVBO()
{
  opengl::cStaticVertexBufferObject vbo;
  context.CreateStaticVertexBufferObject(vbo);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 1.0f;
  const size_t nTextureCoordinates = 1;

  opengl::cGeometryBuilder builder;
  builder.CreateCube(-fWidth, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();

  context.BindStaticVertexBufferObject(vbo);
  context.DrawStaticVertexBufferObjectTriangles(vbo);
  context.UnBindStaticVertexBufferObject(vbo);

  context.DestroyStaticVertexBufferObject(vbo);
}
#endif

}

// ** cPBR

// https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/0d538f002d7d985e4845bfd83545756e5f2942bb/src/6.pbr/2.2.2.ibl_specular_textured/ibl_specular_textured.cpp

bool cPBR::Init(opengl::cContext& context)
{
  Destroy(context);

  context.CreateShader(shader, TEXT("shaders/pbr.vert"), TEXT("shaders/pbr.frag"));
  assert(shader.IsCompiledProgram());

  return LoadAndRenderEquirectangularToCubemap(context);
}

void cPBR::Destroy(opengl::cContext& context)
{
  //context.DestroyStaticVertexBufferObject(bloomToScreenVBO[i]);

  if (fboIrradianceMap.IsValid()) context.DestroyTextureFrameBufferObject(fboIrradianceMap);
  if (fboPrefilterMap.IsValid()) context.DestroyTextureFrameBufferObject(fboPrefilterMap);
  if (fboBRDFLUTTexture.IsValid()) context.DestroyTextureFrameBufferObject(fboBRDFLUTTexture);

  context.DestroyShader(shader);
}

bool cPBR::LoadAndRenderEquirectangularToCubemap(opengl::cContext& context)
{
  // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  opengl::cShader shaderEquirectangularToCubemap;
  context.CreateShader(shaderEquirectangularToCubemap, "shaders/pbr/cubemap.vert", "shaders/pbr/equirectangular_to_cubemap.frag");

  opengl::cShader shaderIrradiance;
  context.CreateShader(shaderIrradiance, "shaders/pbr/cubemap.vert", "shaders/pbr/irradiance_convolution.frag");

  opengl::cShader shaderPrefilter;
  context.CreateShader(shaderPrefilter, "shaders/pbr/cubemap.vert", "shaders/pbr/prefilter.frag");

  opengl::cShader shaderBRDF;
  context.CreateShader(shaderBRDF, "shaders/pbr/brdf.vert", "shaders/pbr/brdf.frag");



  // pbr: load the HDR environment map
  // ---------------------------------
  //const std::string sEnvironmentMapFilePath = "textures/newport_loft.hdr";
  //const std::string sEnvironmentMapFilePath = "textures/white_cliff_top_2k.hdr";
  const std::string sEnvironmentMapFilePath = "textures/christmas_photo_studio_03_2k.hdr";
  voodoo::cImage image;
  if (!image.LoadFromFile(sEnvironmentMapFilePath)) {
    std::cerr<<"Failed to load HDR image \""<<sEnvironmentMapFilePath<<"\""<<std::endl;
    return false;
  }

  assert(image.GetPixelFormat() == voodoo::PIXELFORMAT::RGB32F);

  // The image needs to be upside down
  image.FlipVertically();

  context.CreateTextureFromImageNoMipMaps(fboTextureHDREquirectangularSpheremap, image);

  /*glGenTextures(1, &hdrTexture);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/




  // pbr: setup cubemap to render to and attach to framebuffer
  // ---------------------------------------------------------

  opengl::cTextureFrameBufferObject::FLAGS flagsCubemap;
  flagsCubemap.SetColourBuffer();
  flagsCubemap.SetDepthBuffer();
  flagsCubemap.SetModeCubeMap();

  fboEnvCubemap.SetDoNotUseMipMaps();

  context.CreateTextureFrameBufferObject(fboEnvCubemap, 512, 512, opengl::PIXELFORMAT::RGB16F, flagsCubemap);

/*
  for (unsigned int i = 0; i < 6; ++i)
  {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

  void RenderEquirectangularToCubemap(opengl::cContext& context);


  opengl::cSystem::GetErrorString();

  // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
  // ----------------------------------------------------------------------------------------------
  // ***************
  // HACK: There is something wrong with either these matrix calculations, or the shader, we should be using 90 degrees for full coverage of each cubemap face, 
  // ***************
  //const spitfire::math::cMat4 captureProjection = spitfire::math::cMat4::Perspective(spitfire::math::DegreesToRadians(90.0f), 1.0f, 0.1f, 10.0f);
  const spitfire::math::cMat4 captureProjection = spitfire::math::cMat4::Perspective(spitfire::math::DegreesToRadians(53.0f), 1.0f, 0.1f, 10.0f);
  const spitfire::math::cMat4 captureViews[] = {
    spitfire::math::cMat4::LookAt(spitfire::math::cVec3(0.0f, 0.0f, 0.0f), spitfire::math::cVec3( 1.0f,  0.0f,  0.0f), spitfire::math::cVec3(0.0f, -1.0f,  0.0f)),
    spitfire::math::cMat4::LookAt(spitfire::math::cVec3(0.0f, 0.0f, 0.0f), spitfire::math::cVec3(-1.0f,  0.0f,  0.0f), spitfire::math::cVec3(0.0f, -1.0f,  0.0f)),
    spitfire::math::cMat4::LookAt(spitfire::math::cVec3(0.0f, 0.0f, 0.0f), spitfire::math::cVec3( 0.0f,  1.0f,  0.0f), spitfire::math::cVec3(0.0f,  0.0f,  1.0f)),
    spitfire::math::cMat4::LookAt(spitfire::math::cVec3(0.0f, 0.0f, 0.0f), spitfire::math::cVec3( 0.0f, -1.0f,  0.0f), spitfire::math::cVec3(0.0f,  0.0f, -1.0f)),
    spitfire::math::cMat4::LookAt(spitfire::math::cVec3(0.0f, 0.0f, 0.0f), spitfire::math::cVec3( 0.0f,  0.0f,  1.0f), spitfire::math::cVec3(0.0f, -1.0f,  0.0f)),
    spitfire::math::cMat4::LookAt(spitfire::math::cVec3(0.0f, 0.0f, 0.0f), spitfire::math::cVec3( 0.0f,  0.0f, -1.0f), spitfire::math::cVec3(0.0f, -1.0f,  0.0f))
  };

  context.SetClearColour(spitfire::math::cColour(0.0f, 0.0f, 0.0f));
  context.BeginRenderToCubeMapTexture(fboEnvCubemap);

  // pbr: convert HDR equirectangular environment map to cubemap equivalent
  // ----------------------------------------------------------------------
  context.BindShader(shaderEquirectangularToCubemap);
  context.SetShaderConstant("equirectangularMap", 0);
  context.SetShaderConstant("projection", captureProjection);

  std::cout<<"Binding sphere map"<<std::endl;
  context.BindTexture(0, fboTextureHDREquirectangularSpheremap);

  opengl::cSystem::GetErrorString();

  /*equirectangularToCubemapShader.use();
  equirectangularToCubemapShader.setInt("equirectangularMap", 0);
  equirectangularToCubemapShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);*/

  const opengl::CUBE_MAP_FACE faces[6] = {
    opengl::CUBE_MAP_FACE::POSITIVE_X, // Right
    opengl::CUBE_MAP_FACE::NEGATIVE_X, // Left
    opengl::CUBE_MAP_FACE::POSITIVE_Y, // Top
    opengl::CUBE_MAP_FACE::NEGATIVE_Y, // Bottom
    opengl::CUBE_MAP_FACE::POSITIVE_Z, // Back
    opengl::CUBE_MAP_FACE::NEGATIVE_Z, // Front
  };

  // For debugging
  const spitfire::math::cColour faceColour[6] = {
    spitfire::math::cColour(1.0f, 0.0f, 0.0f), // Red
    spitfire::math::cColour(0.0f, 1.0f, 0.0f), // Green
    spitfire::math::cColour(0.0f, 0.0f, 1.0f), // Blue
    spitfire::math::cColour(1.0f, 1.0f, 0.0f), // Yellow
    spitfire::math::cColour(0.0f, 1.0f, 1.0f), // Cyan
    spitfire::math::cColour(1.0f, 0.0f, 1.0f), // Magenta
  };

  for (size_t i = 0; i < 6; i++) {
    context.SetShaderConstant("view", captureViews[i]);

    context.SetClearColour(faceColour[i]);
    context.BeginRenderToCubeMapTextureFace(fboEnvCubemap, faces[i]);

    opengl::cSystem::GetErrorString();

    // TODO: We should probably invert the cube coordinates so that we don't have to disable culling?
    context.DisableCulling();

    renderCube();

    context.EnableCulling();

    opengl::cSystem::GetErrorString();
  }

  context.UnBindTexture(0, fboTextureHDREquirectangularSpheremap);

  context.UnBindShader(shaderEquirectangularToCubemap);

  opengl::cSystem::GetErrorString();

  context.EndRenderToTexture(fboEnvCubemap);
  context.SetClearColour(spitfire::math::cColour(0.0f, 0.0f, 0.0f));

  opengl::cSystem::GetErrorString();

  // Then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
  fboEnvCubemap.GenerateMipMaps();


  /*unsigned int captureFBO;
  unsigned int captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  // Render to framebuffer
  glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

  for (unsigned int i = 0; i < 6; ++i)
  {
      equirectangularToCubemapShader.setMat4("view", captureViews[i]);

    ... context.BeginRenderToCubeMapTextureFace(cTextureFrameBufferObjectRef pTexture, CUBE_MAP_FACE face);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      renderCube();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);*/



  opengl::cSystem::GetErrorString();



  // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
  // --------------------------------------------------------------------------------
  opengl::cTextureFrameBufferObject::FLAGS flagsIrradianceMap;
  flagsIrradianceMap.SetColourBuffer();
  flagsIrradianceMap.SetDepthBuffer();
  flagsIrradianceMap.SetModeCubeMap();

  context.CreateTextureFrameBufferObject(fboIrradianceMap, 32, 32, opengl::PIXELFORMAT::RGB16F, flagsIrradianceMap);

  /*unsigned int irradianceMap;
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (unsigned int i = 0; i < 6; ++i)
  {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);*/

  opengl::cSystem::GetErrorString();

  context.BeginRenderToCubeMapTexture(fboIrradianceMap);

  // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
  // -----------------------------------------------------------------------------
  context.BindShader(shaderIrradiance);
  context.SetShaderConstant("environmentMap", 0);
  context.SetShaderConstant("projection", captureProjection);

  context.BindTextureCubeMap(0, fboEnvCubemap);

  for (size_t i = 0; i < 6; i++) {
    context.SetShaderConstant("view", captureViews[i]);

    context.BeginRenderToCubeMapTextureFace(fboIrradianceMap, faces[i]);

    context.DisableCulling();

    renderCube();

    context.EnableCulling();
  }

  context.UnBindTextureCubeMap(0, fboEnvCubemap);
  context.UnBindShader(shaderIrradiance);

  context.EndRenderToTexture(fboIrradianceMap);

  /*irradianceShader.use();
  irradianceShader.setInt("environmentMap", 0);
  irradianceShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i)
  {
      irradianceShader.setMat4("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      renderCube();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);*/


  opengl::cSystem::GetErrorString();




  // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
  // --------------------------------------------------------------------------------
  opengl::cTextureFrameBufferObject::FLAGS flagsPrefilterMap;
  flagsPrefilterMap.SetColourBuffer();
  flagsPrefilterMap.SetDepthBuffer();
  flagsPrefilterMap.SetModeCubeMap();

  context.CreateTextureFrameBufferObject(fboPrefilterMap, 128, 128, opengl::PIXELFORMAT::RGB16F, flagsPrefilterMap);

  // Generate mipmaps for the cubemap so OpenGL automatically allocates the required memory
  fboPrefilterMap.GenerateMipMaps();

  /*unsigned int prefilterMap;
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (unsigned int i = 0; i < 6; ++i)
  {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);*/


  opengl::cSystem::GetErrorString();



  // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
  // ----------------------------------------------------------------------------------------------------
  context.BeginRenderToCubeMapTexture(fboPrefilterMap);

  context.BindShader(shaderPrefilter);
  context.SetShaderConstant("environmentMap", 0);
  context.SetShaderConstant("projection", captureProjection);

  context.BindTextureCubeMap(0, fboEnvCubemap);

  // NOTE: This is a bit weird because we want to render each cube face with each mip map level

  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // Resize framebuffer according to mip-level size
    unsigned int mipWidth = 128 * std::pow(0.5, mip);
    unsigned int mipHeight = 128 * std::pow(0.5, mip);
    glBindRenderbuffer(GL_RENDERBUFFER, fboPrefilterMap.uiFBODepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, mipWidth, mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    context.SetShaderConstant("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i) {
      context.SetClearColour(faceColour[i]);

      // Bind the actual face we want to render to
      #if 1
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fboPrefilterMap.GetTexture(), mip);
      glClearColor(faceColour[i].r, faceColour[i].g, faceColour[i].b, faceColour[i].a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      #else
      context.BeginRenderToCubeMapTextureFace(fboPrefilterMap, faces[i]);
      #endif

      context.SetShaderConstant("view", captureViews[i]);

      context.DisableCulling();

      renderCube();

      context.EnableCulling();
    }
  }

  context.UnBindTextureCubeMap(0, fboEnvCubemap);

  context.UnBindShader(shaderPrefilter);

  context.EndRenderToTexture(fboPrefilterMap);


  /*
  prefilterShader.use();
  prefilterShader.setInt("environmentMap", 0);
  prefilterShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
  {
      // reisze framebuffer according to mip-level size.
      unsigned int mipWidth = 128 * std::pow(0.5, mip);
      unsigned int mipHeight = 128 * std::pow(0.5, mip);
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
      glViewport(0, 0, mipWidth, mipHeight);

      float roughness = (float)mip / (float)(maxMipLevels - 1);
      prefilterShader.setFloat("roughness", roughness);
      for (unsigned int i = 0; i < 6; ++i)
      {
          prefilterShader.setMat4("view", captureViews[i]);
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          renderCube();
      }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);*/



  opengl::cSystem::GetErrorString();

  // pbr: generate a 2D LUT from the BRDF equations used.
  // ----------------------------------------------------

  // TODO: Move this to a second function
  opengl::cTextureFrameBufferObject::FLAGS flagsBRDF;
  flagsBRDF.SetColourBuffer();
  flagsBRDF.SetDepthBuffer();

  context.CreateTextureFrameBufferObjectNoMipMaps(fboBRDFLUTTexture, 512, 512, opengl::PIXELFORMAT::RGB16F, flagsBRDF);

  {
    context.BeginRenderToTexture(fboBRDFLUTTexture);
    context.BindShader(shaderBRDF);

    renderQuad();

    context.UnBindShader(shaderBRDF);
    context.EndRenderToTexture(fboBRDFLUTTexture);
  }

  /*unsigned int brdfLUTTexture;
  glGenTextures(1, &brdfLUTTexture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

  glViewport(0, 0, 512, 512);
  brdfShader.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderQuad();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

  opengl::cSystem::GetErrorString();

  //context.DestroyTexture(fboTextureHDREquirectangularSpheremap);
  //context.DestroyTextureFrameBufferObject(fboEnvCubemap);

  context.DestroyShader(shaderEquirectangularToCubemap);
  context.DestroyShader(shaderIrradiance);
  context.DestroyShader(shaderPrefilter);
  context.DestroyShader(shaderBRDF);

  return true;
}

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
#include <spitfire/util/timer.h>
#include <spitfire/util/thread.h>

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

// Breathe headers
#include <breathe/render/model/cFileFormatOBJ.h>
#include <breathe/render/model/cStatic.h>

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

// Application headers
#include "grass.h"
#include "main.h"

bool cGrass::Init(opengl::cContext& context)
{
  context.CreateShader(shader, TEXT("shaders/grass.vert"), TEXT("shaders/grass.frag"));
  context.CreateTexture(texture, TEXT("textures/grass.png"));

  // We actually need to clamp to edge, otherwise the texture coordinates at the top of the grass wrap around and show pixels from the bottom of the texture
  context.BindTexture(0, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  context.UnBindTexture(0, texture);


  // Create the grass geometry
  const float fOneOver255 = 1.0f / 255.0f;

  opengl::cGeometryDataPtr pGrassGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v3_n3_t2_c4 builderGrass(*pGrassGeometryDataPtr);

  const spitfire::math::cVec3 minGrass(-0.5f, 0.0f, 0.0f);
  const spitfire::math::cVec3 maxGrass(0.5f, 1.0f, 0.0f);
  const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);

  const spitfire::math::cVec3 axisY(0.0f, 1.0f, 0.0f);

  spitfire::math::cScopedPredictableRandom randomGenerator(4754);

  // We create our geometry by creating a grid of grass at a set scale, then jittering the position of every bit of grass slightly -0.5..0.5
  // It's not the best, but is fast and simple

  // The scale of the grid
  const spitfire::math::cVec2 gridScale(0.3f, 0.3f);

  const size_t gridRows = 40;
  const size_t gridColumns = 40;
  for (size_t y = 0; y < gridRows; y++) {
    for (size_t x = 0; x < gridColumns; x++) {
      const spitfire::math::cVec2 randomOffset2D(-0.5f + randomGenerator.GetRandomNumber0To1(), -0.5f +  + randomGenerator.GetRandomNumber0To1());
      const spitfire::math::cVec2 position2D(gridScale * (spitfire::math::cVec2(x, y) + randomOffset2D));
      const spitfire::math::cVec3 position = spitfire::math::cVec3(position2D.x, 0.0f, position2D.y);

      const float fRotationDegreesOfGrass = randomGenerator.GetRandomNumber0To1() * 360.0f;

      const spitfire::math::cColour colourBase(54.0f * fOneOver255, 124.0f * fOneOver255, 65.0f * fOneOver255);
      const spitfire::math::cColour colour(colourBase.r + (0.1f * (-0.5f + randomGenerator.GetRandomNumber0To1())), colourBase.g + (0.1f * (-0.5f + randomGenerator.GetRandomNumber0To1())), colourBase.b + (0.1f * (-0.5f + randomGenerator.GetRandomNumber0To1())));

      // Create 3 billboards at 60 degree increments
      /* From above they will look something like this:
        \|/
        -*-
        /|\
      */
      for (size_t i = 0; i < 3; i++) {
        const float fRotationDegrees = fRotationDegreesOfGrass + (float(i + 1) * 60.0f);

        // The rotation for this billboard of grass
        spitfire::math::cQuaternion rotationY;
        rotationY.SetFromAxisAngleDegrees(axisY, fRotationDegrees);

        spitfire::math::cMat4 matRotation;
        matRotation.SetRotation(rotationY);

        const spitfire::math::cVec3 point[4] = {
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minGrass.x, minGrass.y, minGrass.z)),
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxGrass.x, minGrass.y, minGrass.z)),
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxGrass.x, maxGrass.y, minGrass.z)),
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minGrass.x, maxGrass.y, minGrass.z)),
        };

        const spitfire::math::cVec3 normalRotated = matRotation.GetRotatedVec3(normal);

        // Front face
        builderGrass.PushBack(point[0], normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
        builderGrass.PushBack(point[1], normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
        builderGrass.PushBack(point[2], normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
        builderGrass.PushBack(point[2], normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
        builderGrass.PushBack(point[3], normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);
        builderGrass.PushBack(point[0], normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);

        // Back face
        builderGrass.PushBack(point[0], -normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
        builderGrass.PushBack(point[2], -normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
        builderGrass.PushBack(point[1], -normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
        builderGrass.PushBack(point[2], -normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
        builderGrass.PushBack(point[0], -normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
        builderGrass.PushBack(point[3], -normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);
      }
    }
  }


  context.CreateStaticVertexBufferObject(vbo);

  vbo.SetData(pGrassGeometryDataPtr);
  vbo.Compile();

  return true;
}

void cGrass::Destroy(opengl::cContext& context)
{
  context.DestroyStaticVertexBufferObject(vbo);
  context.DestroyTexture(texture);
  context.DestroyShader(shader);
}

void cGrass::Resize(cApplication& application, opengl::cContext& context)
{
}

void cGrass::Update(opengl::cContext& context, spitfire::durationms_t currentTime)
{
  // ... update uniform on shader
}

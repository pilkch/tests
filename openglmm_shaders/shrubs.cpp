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
#include <libopenglmm/cFont.h>
#include <libopenglmm/cGeometry.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cSystem.h>
#include <libopenglmm/cTexture.h>
#include <libopenglmm/cVertexBufferObject.h>
#include <libopenglmm/cWindow.h>

// Application headers
#include "shrubs.h"
#include "main.h"

bool cShrubs::Init(opengl::cContext& context)
{
  context.CreateShader(shader, TEXT("shaders/shrub.vert"), TEXT("shaders/shrub.frag"));
  context.CreateTexture(texture, TEXT("textures/shrub.png"));

  // We actually need to clamp to edge, otherwise the texture coordinates at the top of the shrub wrap around and show pixels from the bottom of the texture
  context.BindTexture(0, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  context.UnBindTexture(0, texture);


  // Create the shrub geometry
  const float fOneOver255 = 1.0f / 255.0f;

  opengl::cGeometryDataPtr pShrubGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v3_n3_t2_c4 builderShrub(*pShrubGeometryDataPtr);

  const spitfire::math::cVec3 minShrub(-0.5f, 0.0f, 0.0f);
  const spitfire::math::cVec3 maxShrub(0.5f, 1.0f, 0.0f);
  const spitfire::math::cVec3 normal(0.0f, 0.0f, 1.0f);

  const spitfire::math::cVec3 axisY(0.0f, 1.0f, 0.0f);

  spitfire::math::cScopedPredictableRandom randomGenerator(3843);

  // We create our geometry by creating a grid of shrubs at a set scale, then jittering the position of every bit of shrub slightly -0.5..0.5
  // It's not the best, but is fast and simple

  // The scale of the grid
  const spitfire::math::cVec2 gridScale(0.3f, 0.3f);

  const size_t gridRows = 40;
  const size_t gridColumns = 40;

  // The middle of the shrubs field
  const spitfire::math::cVec3 center(spitfire::math::cVec3(20.0f, 0.0f, 20.0f) + (-0.5f * spitfire::math::cVec3(gridScale.x * float(gridRows), 0.0f, gridScale.y * float(gridColumns))));

  for (size_t y = 0; y < gridRows; y++) {
    for (size_t x = 0; x < gridColumns; x++) {
      const spitfire::math::cVec2 randomOffset2D(-0.5f + randomGenerator.GetRandomNumber0To1(), -0.5f +  + randomGenerator.GetRandomNumber0To1());
      const spitfire::math::cVec2 position2D(gridScale * (spitfire::math::cVec2(x, y) + randomOffset2D));
      const spitfire::math::cVec3 position = center + spitfire::math::cVec3(position2D.x, 0.0f, position2D.y);

      const float fRotationDegreesOfShrub = randomGenerator.GetRandomNumber0To1() * 360.0f;

      const spitfire::math::cColour colourBase(54.0f * fOneOver255, 124.0f * fOneOver255, 65.0f * fOneOver255);
      const spitfire::math::cColour colour(colourBase.r + (-0.1f * (0.2f + randomGenerator.GetRandomNumber0To1())), colourBase.g + (-0.1f * (0.2f + randomGenerator.GetRandomNumber0To1())), colourBase.b + (-0.1f * (0.2f + randomGenerator.GetRandomNumber0To1())));

      // Create 3 billboards at 60 degree increments
      /* From above they will look something like this:
        \|/
        -*-
        /|\
      */
      for (size_t i = 0; i < 3; i++) {
        const float fRotationDegrees = fRotationDegreesOfShrub + (float(i + 1) * 60.0f);

        // The rotation for this billboard of shrub
        spitfire::math::cQuaternion rotationY;
        rotationY.SetFromAxisAngleDegrees(axisY, fRotationDegrees);

        spitfire::math::cMat4 matRotation;
        matRotation.SetRotation(rotationY);

        const spitfire::math::cVec3 point[4] = {
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minShrub.x, minShrub.y, minShrub.z)),
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxShrub.x, minShrub.y, minShrub.z)),
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxShrub.x, maxShrub.y, minShrub.z)),
          position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minShrub.x, maxShrub.y, minShrub.z)),
        };

        // NOTE: We pack the RGB colour of the shrub and the wind influence into the colour
        const float fWindInfluence0To1 = 0.7f + (randomGenerator.GetRandomNumber0To1() * 0.3f); // We introduce a little bit of randomness into the amount wind influence
        const spitfire::math::cColour4 colourAndWindInfluence[4] = {
          spitfire::math::cColour4(colour.r, colour.g, colour.b, 0.0f),
          spitfire::math::cColour4(colour.r, colour.g, colour.b, 0.0f),
          spitfire::math::cColour4(colour.r, colour.g, colour.b, fWindInfluence0To1),
          spitfire::math::cColour4(colour.r, colour.g, colour.b, fWindInfluence0To1),
        };

        const spitfire::math::cVec3 normalRotated = matRotation.GetRotatedVec3(normal);

        // NOTE: We only create one side of the geometry but we render with culling disabled so that we render both sides
        builderShrub.PushBack(point[0], normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colourAndWindInfluence[0]);
        builderShrub.PushBack(point[1], normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colourAndWindInfluence[1]);
        builderShrub.PushBack(point[2], normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colourAndWindInfluence[2]);
        builderShrub.PushBack(point[2], normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colourAndWindInfluence[2]);
        builderShrub.PushBack(point[3], normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colourAndWindInfluence[3]);
        builderShrub.PushBack(point[0], normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colourAndWindInfluence[0]);
      }
    }
  }


  context.CreateStaticVertexBufferObject(vbo);

  vbo.SetData(pShrubGeometryDataPtr);
  vbo.Compile();

  return true;
}

void cShrubs::Destroy(opengl::cContext& context)
{
  context.DestroyStaticVertexBufferObject(vbo);
  context.DestroyTexture(texture);
  context.DestroyShader(shader);
}

void cShrubs::Resize(cApplication& application, opengl::cContext& context)
{
}

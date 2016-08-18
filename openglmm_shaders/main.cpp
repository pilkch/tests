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
#include <SDL2/SDL_image.h>

// Spitfire headers
#include <spitfire/spitfire.h>
#include <spitfire/util/timer.h>
#include <spitfire/util/thread.h>

// TODO: GET A HEBE STATUE MODEL

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
#include "main.h"

// ** cApplication

cApplication::cApplication() :
  fFPS(0.0f),

  bReloadShaders(false),
  bUpdateShaderConstants(true),

  bIsMovingForward(false),
  bIsMovingLeft(false),
  bIsMovingRight(false),
  bIsMovingBackward(false),

  moveLightForward(SDLK_t),
  moveLightBack(SDLK_g),
  moveLightLeft(SDLK_f),
  moveLightRight(SDLK_h),

  bIsCameraAtLightSource(false),

  bIsFocalLengthIncrease(false),
  bIsFocalLengthDecrease(false),
  bIsFStopIncrease(false),
  bIsFStopDecrease(false),

  bIsDirectionalLightOn(true),
  bIsPointLightOn(true),
  bIsSpotLightOn(true),
  lightPointPosition(-2.3f, 7.0f, 0.8f),

  bIsRotating(true),
  bIsWireframe(false),

  bIsDOFBokeh(true),
  bIsLensFlareDirt(true),
  bIsHDR(true),
  bDebugShowFlareOnly(false),
  bIsSplitScreenSimplePostEffectShaders(true),

  bIsDone(false),

  pWindow(nullptr),
  pContext(nullptr),

  bSimplePostRenderDirty(false),

  colourBlindMode(COLOUR_BLIND_MODE::PROTANOPIA)
{
  // Set our main thread
  spitfire::util::SetMainThread();

  // Set up our time variables
  spitfire::util::TimeInit();
}

opengl::cResolution cApplication::GetResolution() const
{
  return resolution;
}

void cApplication::CreateText()
{
  assert(font.IsValid());

  // Destroy any existing VBO
  pContext->DestroyStaticVertexBufferObject(textVBO);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v2_c4_t2 builder(*pGeometryDataPtr);

  std::list<spitfire::string_t> lines;
  lines.push_back(spitfire::string_t(TEXT("FPS: ")) + spitfire::string::ToString(int(fFPS)));
  lines.push_back(TEXT(""));

  lines.push_back(spitfire::string_t(TEXT("Rotating: ")) + (bIsRotating ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Wireframe: ")) + (bIsWireframe ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Direction light: ")) + (bIsDirectionalLightOn ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Point light: ")) + (bIsPointLightOn ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Spotlight: ")) + (bIsSpotLightOn ? TEXT("On") : TEXT("Off")));
  lines.push_back(TEXT(""));

  lines.push_back(spitfire::string_t(TEXT("DOF bokeh: ")) + (bIsDOFBokeh ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Focal length:") + spitfire::string::ToString(dofBokeh.GetFocalLengthmm()) + TEXT("mm")));
  lines.push_back(spitfire::string_t(TEXT("f stops:") + spitfire::string::ToString(dofBokeh.GetFStop())));

  lines.push_back(TEXT(""));
  lines.push_back(spitfire::string_t(TEXT("HDR: ")) + (bIsHDR ? TEXT("On") : TEXT("Off")));
  const float fExposure = hdr.GetExposure();
  lines.push_back(spitfire::string_t(TEXT("Exposure: ")) + spitfire::string::ToString(fExposure));
  lines.push_back(spitfire::string_t(TEXT("Lens Flare and Dirt: ")) + (bIsLensFlareDirt ? TEXT("On") : TEXT("Off")));
  lines.push_back(spitfire::string_t(TEXT("Debug show lens flare and dirt only: ")) + (bDebugShowFlareOnly ? TEXT("On") : TEXT("Off")));

  // Post render shaders
  if (GetActiveSimplePostRenderShadersCount() == 0) {
    lines.push_back(TEXT("Post render effects: "));
    lines.push_back(TEXT("None"));
  } else {
    lines.push_back(spitfire::string_t(TEXT("Split post render effects: ")) + (bIsSplitScreenSimplePostEffectShaders ? TEXT("On") : TEXT("Off")));
    lines.push_back(TEXT("Post render effects: "));
    // Print the name for each post render shader that is turned on
    const size_t n = simplePostRenderShaders.size();
    for (size_t i = 0; i < n; i++) {
      if (simplePostRenderShaders[i].bOn) lines.push_back(simplePostRenderShaders[i].sName);
    }

    // Print the colour blind mode
    if (IsColourBlindSimplePostRenderShaderEnabled()) {
      if (colourBlindMode == COLOUR_BLIND_MODE::PROTANOPIA) lines.push_back(TEXT("Protanopia"));
      else if (colourBlindMode == COLOUR_BLIND_MODE::DEUTERANOPIA) lines.push_back(TEXT("Deuteranopia"));
      else lines.push_back(TEXT("Tritanopia"));
    }
  }

  // Add our lines of text
  const spitfire::math::cColour red(1.0f, 0.0f, 0.0f);
  float y = 0.0f;
  std::list<spitfire::string_t>::const_iterator iter(lines.begin());
  const std::list<spitfire::string_t>::const_iterator iterEnd(lines.end());
  while (iter != iterEnd) {
    font.PushBack(builder, *iter, red, spitfire::math::cVec2(0.0f, y));
    y += 0.04f;

    iter++;
  }

  textVBO.SetData(pGeometryDataPtr);

  textVBO.Compile2D();
}

void cApplication::CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreatePlane(fWidth, fDepth, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreatePlane(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fSizeMeters, float fTextureWidthWorldSpaceMeters)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fTextureUV = fSizeMeters / fTextureWidthWorldSpaceMeters;

  opengl::cGeometryBuilder builder;
  builder.CreatePlane(fSizeMeters, fSizeMeters, fTextureUV, fTextureUV, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateCube(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateCube(fWidth, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateBox(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fWidth = 2.0f;
  const float fDepth = 1.0f;
  const float fHeight = 1.0f;

  opengl::cGeometryBuilder builder;
  builder.CreateBox(fWidth, fDepth, fHeight, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateSphere(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates, float fRadius)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const size_t nSegments = 30;

  opengl::cGeometryBuilder builder;
  builder.CreateSphere(fRadius, nSegments, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateTeapot(opengl::cStaticVertexBufferObject& vbo, size_t nTextureCoordinates)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fRadius = 1.0f;
  const size_t nSegments = 20;

  opengl::cGeometryBuilder builder;
  builder.CreateTeapot(fRadius, nSegments, *pGeometryDataPtr, nTextureCoordinates);

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateGear(opengl::cStaticVertexBufferObject& vbo)
{
}


class cGeometryBuilder_v3_n3_t2_tangent4
{
public:
  explicit cGeometryBuilder_v3_n3_t2_tangent4(opengl::cGeometryData& data);

  void PushBackQuad(
    const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2, const spitfire::math::cVec3& vertex3,
    const spitfire::math::cVec3& normal,
    const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2, const spitfire::math::cVec2& textureCoord3
  );

private:
  spitfire::math::cVec4 CalculateTangentVectorForTriangle(
    const spitfire::math::cVec3& pos1, const spitfire::math::cVec3& pos2, const spitfire::math::cVec3& pos3,
    const spitfire::math::cVec2& texCoord1, const spitfire::math::cVec2& texCoord2, const spitfire::math::cVec2& texCoord3,
    const spitfire::math::cVec3& normal
  ) const;

  void PushBackTriangle(
    const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2,
    const spitfire::math::cVec3& normal,
    const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2,
    const spitfire::math::cVec4& tangent
  );
  void PushBackVertex(const spitfire::math::cVec3& vertex, const spitfire::math::cVec3& normal, const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec4& tangent);

  opengl::cGeometryData& data;
};

// Given the 3 vertices (position and texture coordinates) of a triangle
// calculate and return the triangle's tangent vector.
// http://www.dhpoware.com/demos/gl3ParallaxNormalMapping.html
spitfire::math::cVec4 cGeometryBuilder_v3_n3_t2_tangent4::CalculateTangentVectorForTriangle(
  const spitfire::math::cVec3& pos1, const spitfire::math::cVec3& pos2, const spitfire::math::cVec3& pos3,
  const spitfire::math::cVec2& texCoord1, const spitfire::math::cVec2& texCoord2, const spitfire::math::cVec2& texCoord3,
  const spitfire::math::cVec3& normal
) const
{
  // Create 2 vectors in object space.
  //
  // edge1 is the vector from vertex positions pos1 to pos2.
  // edge2 is the vector from vertex positions pos1 to pos3.
  spitfire::math::cVec3 edge1(pos2 - pos1);
  spitfire::math::cVec3 edge2(pos3 - pos1);

  edge1.Normalise();
  edge2.Normalise();

  // Create 2 vectors in tangent (texture) space that point in the same
  // direction as edge1 and edge2 (in object space).
  //
  // texEdge1 is the vector from texture coordinates texCoord1 to texCoord2.
  // texEdge2 is the vector from texture coordinates texCoord1 to texCoord3.
  spitfire::math::cVec2 texEdge1(texCoord2 - texCoord1);
  spitfire::math::cVec2 texEdge2(texCoord3 - texCoord1);

  texEdge1.Normalise();
  texEdge2.Normalise();

  // These 2 sets of vectors form the following system of equations:
  //
  //  edge1 = (texEdge1.x * tangent) + (texEdge1.y * bitangent)
  //  edge2 = (texEdge2.x * tangent) + (texEdge2.y * bitangent)
  //
  // Using matrix notation this system looks like:
  //
  //  [ edge1 ]     [ texEdge1.x  texEdge1.y ]  [ tangent   ]
  //  [       ]  =  [                        ]  [           ]
  //  [ edge2 ]     [ texEdge2.x  texEdge2.y ]  [ bitangent ]
  //
  // The solution is:
  //
  //  [ tangent   ]        1     [ texEdge2.y  -texEdge1.y ]  [ edge1 ]
  //  [           ]  =  -------  [                         ]  [       ]
  //  [ bitangent ]      det A   [-texEdge2.x   texEdge1.x ]  [ edge2 ]
  //
  //  where:
  //        [ texEdge1.x  texEdge1.y ]
  //    A = [                        ]
  //        [ texEdge2.x  texEdge2.y ]
  //
  //    det A = (texEdge1.x * texEdge2.y) - (texEdge1.y * texEdge2.x)
  //
  // From this solution the tangent space basis vectors are:
  //
  //    tangent = (1 / det A) * ( texEdge2.y * edge1 - texEdge1.y * edge2)
  //  bitangent = (1 / det A) * (-texEdge2.x * edge1 + texEdge1.x * edge2)
  //     normal = cross(tangent, bitangent)

  float det = (texEdge1.x * texEdge2.y) - (texEdge1.y * texEdge2.x);

  spitfire::math::cVec3 t;
  spitfire::math::cVec3 b;
  if (spitfire::math::IsApproximatelyZero(det)) {
    t.Set(1.0f, 0.0f, 0.0f);
    b.Set(0.0f, 1.0f, 0.0f);
  } else {
    det = 1.0f / det;

    t.x = ((texEdge2.y * edge1.x) - (texEdge1.y * edge2.x)) * det;
    t.y = ((texEdge2.y * edge1.y) - (texEdge1.y * edge2.y)) * det;
    t.z = ((texEdge2.y * edge1.z) - (texEdge1.y * edge2.z)) * det;

    b.x = ((-texEdge2.x * edge1.x) + (texEdge1.x * edge2.x)) * det;
    b.y = ((-texEdge2.x * edge1.y) + (texEdge1.x * edge2.y)) * det;
    b.z = ((-texEdge2.x * edge1.z) + (texEdge1.x * edge2.z)) * det;

    t.Normalise();
    b.Normalise();
  }

  // Calculate the handedness of the local tangent space.
  // The bitangent vector is the cross product between the triangle face
  // normal vector and the calculated tangent vector. The resulting bitangent
  // vector should be the same as the bitangent vector calculated from the
  // set of linear equations above. If they point in different directions
  // then we need to invert the cross product calculated bitangent vector. We
  // store this scalar multiplier in the tangent vector's 'w' component so
  // that the correct bitangent vector can be generated in the normal mapping
  // shader's vertex shader.

  const spitfire::math::cVec3 bitangent = normal.CrossProduct(t);
  const float handedness = (bitangent.DotProduct(b) < 0.0f) ? -1.0f : 1.0f;

  spitfire::math::cVec4 tangent;

  tangent.x = t.x;
  tangent.y = t.y;
  tangent.z = t.z;
  tangent.w = handedness;

  return tangent;
}

inline cGeometryBuilder_v3_n3_t2_tangent4::cGeometryBuilder_v3_n3_t2_tangent4(opengl::cGeometryData& _data) :
  data(_data)
{
  data.nVerticesPerPoint = 3;
  data.nNormalsPerPoint = 3;
  data.nTextureCoordinatesPerPoint = 2;
  data.nFloatUserData0PerPoint = 4;
}

void cGeometryBuilder_v3_n3_t2_tangent4::PushBackVertex(const spitfire::math::cVec3& vertex, const spitfire::math::cVec3& normal, const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec4& tangent)
{
  data.vertices.push_back(vertex.x);
  data.vertices.push_back(vertex.y);
  data.vertices.push_back(vertex.z);
  data.vertices.push_back(normal.x);
  data.vertices.push_back(normal.y);
  data.vertices.push_back(normal.z);
  data.vertices.push_back(textureCoord0.x);
  data.vertices.push_back(textureCoord0.y);
  data.vertices.push_back(tangent.x);
  data.vertices.push_back(tangent.y);
  data.vertices.push_back(tangent.z);
  data.vertices.push_back(tangent.w);
  data.nVertexCount++;
}

void cGeometryBuilder_v3_n3_t2_tangent4::PushBackTriangle(
  const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2,
  const spitfire::math::cVec3& normal,
  const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2,
  const spitfire::math::cVec4& tangent
)
{
  PushBackVertex(vertex0, normal, textureCoord0, tangent);
  PushBackVertex(vertex1, normal, textureCoord1, tangent);
  PushBackVertex(vertex2, normal, textureCoord2, tangent);
}

void cGeometryBuilder_v3_n3_t2_tangent4::PushBackQuad(
  const spitfire::math::cVec3& vertex0, const spitfire::math::cVec3& vertex1, const spitfire::math::cVec3& vertex2, const spitfire::math::cVec3& vertex3,
  const spitfire::math::cVec3& normal,
  const spitfire::math::cVec2& textureCoord0, const spitfire::math::cVec2& textureCoord1, const spitfire::math::cVec2& textureCoord2, const spitfire::math::cVec2& textureCoord3
)
{
  const spitfire::math::cVec4 tangent = CalculateTangentVectorForTriangle(
    vertex0, vertex1, vertex2,
    textureCoord0, textureCoord1, textureCoord2,
    normal
  );

  PushBackTriangle(vertex0, vertex1, vertex2, normal, textureCoord0, textureCoord1, textureCoord2, tangent);
  PushBackTriangle(vertex2, vertex3, vertex0, normal, textureCoord2, textureCoord3, textureCoord0, tangent);
}

void cApplication::CreateNormalMappedCube()
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  // Build a cube with an extra field for the tangent on each vertex
  cGeometryBuilder_v3_n3_t2_tangent4 builder(*pGeometryDataPtr);

  const float fWidth = 1.0f;

  const spitfire::math::cVec3 vMin(-fWidth * 0.5f, -fWidth * 0.5f, -fWidth * 0.5f);
  const spitfire::math::cVec3 vMax(fWidth * 0.5f, fWidth * 0.5f, fWidth * 0.5f);

  // Upper Square
  builder.PushBackQuad(
    spitfire::math::cVec3(vMin.x, vMax.y, vMin.z), spitfire::math::cVec3(vMin.x, vMax.y, vMax.z), spitfire::math::cVec3(vMax.x, vMax.y, vMax.z), spitfire::math::cVec3(vMax.x, vMax.y, vMin.z),
    spitfire::math::cVec3(0.0f, 1.0f, 0.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  // Bottom Square
  builder.PushBackQuad(
    spitfire::math::cVec3(vMax.x, vMin.y, vMin.z), spitfire::math::cVec3(vMax.x, vMin.y, vMax.z), spitfire::math::cVec3(vMin.x, vMin.y, vMax.z), spitfire::math::cVec3(vMin.x, vMin.y, vMin.z),
    spitfire::math::cVec3(0.0f, -1.0f, 0.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  // Side Squares
  builder.PushBackQuad(
    spitfire::math::cVec3(vMin.x, vMin.y, vMin.z), spitfire::math::cVec3(vMin.x, vMin.y, vMax.z), spitfire::math::cVec3(vMin.x, vMax.y, vMax.z), spitfire::math::cVec3(vMin.x, vMax.y, vMin.z),
    spitfire::math::cVec3(-1.0f, 0.0f, 0.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  builder.PushBackQuad(
    spitfire::math::cVec3(vMax.x, vMin.y, vMax.z), spitfire::math::cVec3(vMax.x, vMin.y, vMin.z), spitfire::math::cVec3(vMax.x, vMax.y, vMin.z), spitfire::math::cVec3(vMax.x, vMax.y, vMax.z),
    spitfire::math::cVec3(1.0f, 0.0f, 0.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  // Front
  builder.PushBackQuad(
    spitfire::math::cVec3(vMax.x, vMin.y, vMin.z), spitfire::math::cVec3(vMin.x, vMin.y, vMin.z), spitfire::math::cVec3(vMin.x, vMax.y, vMin.z), spitfire::math::cVec3(vMax.x, vMax.y, vMin.z),
    spitfire::math::cVec3(0.0f, 0.0f, -1.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  // Back
  builder.PushBackQuad(
    spitfire::math::cVec3(vMin.x, vMin.y, vMax.z), spitfire::math::cVec3(vMax.x, vMin.y, vMax.z), spitfire::math::cVec3(vMax.x, vMax.y, vMax.z), spitfire::math::cVec3(vMin.x, vMax.y, vMax.z),
    spitfire::math::cVec3(0.0f, 0.0f, 1.0f),
    spitfire::math::cVec2(1.0f, 0.0f), spitfire::math::cVec2(1.0f, 1.0f), spitfire::math::cVec2(0.0f, 1.0f), spitfire::math::cVec2(0.0f, 0.0f)
  );

  parallaxNormalMap.vbo.SetData(pGeometryDataPtr);

  parallaxNormalMap.vbo.Compile();
}

void cApplication::CreateTeapotVBO()
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder builder;

  const float fRadius = 0.3f;
  const size_t nSegments = 20;
  builder.CreateTeapot(fRadius, nSegments, *pGeometryDataPtr, 1);

  staticVertexBufferObjectLargeTeapot.SetData(pGeometryDataPtr);

  staticVertexBufferObjectLargeTeapot.Compile();
}

#ifdef BUILD_LARGE_STATUE_MODEL
void cApplication::CreateStatueVBO()
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  breathe::render::model::cStaticModel model;

  breathe::render::model::cFileFormatOBJ loader;
  if (!loader.Load(TEXT("models/venus.obj"), model)) {
    LOG("Failed to load obj file");
    return;
  }

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

  // The obj file is giant so scale it down as well
  spitfire::math::cMat4 matScale;
  matScale.SetScale(spitfire::math::cVec3(0.1f, 0.1f, 0.1f));

  const spitfire::math::cMat4 matTransform = matScale;

  const size_t nMeshes = model.mesh.size();
  for (size_t iMesh = 0; iMesh < nMeshes; iMesh++) {
    const size_t nVertices = model.mesh[iMesh]->vertices.size() / 3;
    for (size_t iVertex = 0; iVertex < nVertices; iVertex++) {
      builder.PushBack(
        matTransform * spitfire::math::cVec3(model.mesh[iMesh]->vertices[(3 * iVertex)], model.mesh[iMesh]->vertices[(3 * iVertex) + 1], model.mesh[iMesh]->vertices[(3 * iVertex) + 2]),
        spitfire::math::cVec3(model.mesh[iMesh]->normals[(3 * iVertex)], model.mesh[iMesh]->normals[(3 * iVertex) + 1], model.mesh[iMesh]->normals[(3 * iVertex) + 2]),
        spitfire::math::cVec2(model.mesh[iMesh]->textureCoordinates[(2 * iVertex)], model.mesh[iMesh]->textureCoordinates[(2 * iVertex) + 1])
      );
    }
  }

  staticVertexBufferObjectStatue.SetData(pGeometryDataPtr);

  staticVertexBufferObjectStatue.Compile();
}
#endif

void cApplication::CreateScreenRectVariableTextureSizeVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight)
{
  const float fTextureWidth = 1.0f;
  const float fTextureHeight = 1.0f;

  CreateScreenRectVBO(staticVertexBufferObject, fWidth, fHeight, fTextureWidth, fTextureHeight);
}

void cApplication::CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight)
{
  const float fTextureWidth = float(resolution.width);
  const float fTextureHeight = float(resolution.height);

  CreateScreenRectVBO(staticVertexBufferObject, fWidth, fHeight, fTextureWidth, fTextureHeight);
}

void cApplication::CreateScreenRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fVBOWidth, float_t fVBOHeight, float_t fTextureWidth, float_t fTextureHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float_t fHalfWidth = fVBOWidth * 0.5f;
  const float_t fHalfHeight = fVBOHeight * 0.5f;
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

  staticVertexBufferObject.SetData(pGeometryDataPtr);

  staticVertexBufferObject.Compile2D();
}

void cApplication::CreateGuiRectangle(opengl::cStaticVertexBufferObject& vbo, size_t nTextureWidth, size_t nTextureHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fTextureWidth = float(nTextureWidth);
  const float fTextureHeight = float(nTextureHeight);

  const float fRatioWidthOverHeight = (fTextureWidth / fTextureHeight);

  const float fWidth = 1.0f;
  const float fHeight = fWidth / fRatioWidthOverHeight;

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec3 vMin(-fHalfWidth, 0.0f, -fHalfHeight);
  const spitfire::math::cVec3 vMax(fHalfWidth, 0.0f, fHalfHeight);
  const spitfire::math::cVec3 vNormal(0.0f, 0.0f, -1.0f);

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.z, 0.0f), vNormal, spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.z, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMax.z, 0.0f), vNormal, spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMin.z, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.z, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.z, 0.0f), vNormal, spitfire::math::cVec2(0.0f, fTextureHeight));

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateScreenHalfRectVBO(opengl::cStaticVertexBufferObject& staticVertexBufferObject, float_t fWidth, float_t fHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fHalfTextureWidth = 0.5f * float(resolution.width);
  const float fTextureHeight = float(resolution.height);

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec2 vMin(-fHalfWidth, -fHalfHeight);
  const spitfire::math::cVec2 vMax(fHalfWidth, fHalfHeight);

  opengl::cGeometryBuilder_v2_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(fHalfTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMax.y), spitfire::math::cVec2(fHalfTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMin.y), spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(fHalfTextureWidth, fTextureHeight));

  staticVertexBufferObject.SetData(pGeometryDataPtr);

  staticVertexBufferObject.Compile2D();
}

void cApplication::CreateLightBillboard()
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  // NOTE: We use an extra set of texture coordinates on the end to say where the center point of that specific particle is
  opengl::cGeometryBuilder_v3_n3_t2_user3 builder(*pGeometryDataPtr);

  const spitfire::math::cVec3 normal(0.0f, 0.0f, -1.0f);

  const spitfire::math::cVec3 position(0.0f, 0.0f, 0.0f);

  const float fSize = 1.0f;
  const float fWidth = fSize;
  const float fHeight = fSize;

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec3 vMin(-fHalfWidth, -fHalfHeight, 0.0f);
  const spitfire::math::cVec3 vMax(fHalfWidth, fHalfHeight, 0.0f);

  // Add a front facing rectangle
  builder.PushBack(position + spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), normal, spitfire::math::cVec2(0.0f, 0.0f), position);
  builder.PushBack(position + spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), normal, spitfire::math::cVec2(1.0, 1.0), position);
  builder.PushBack(position + spitfire::math::cVec3(vMax.x, vMin.y, 0.0f), normal, spitfire::math::cVec2(0.0f, 1.0), position);
  builder.PushBack(position + spitfire::math::cVec3(vMin.x, vMax.y, 0.0f), normal, spitfire::math::cVec2(1.0, 0.0f), position);
  builder.PushBack(position + spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), normal, spitfire::math::cVec2(1.0, 1.0), position);
  builder.PushBack(position + spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), normal, spitfire::math::cVec2(0.0f, 0.0f), position);

  light.vbo.SetData(pGeometryDataPtr);

  light.vbo.Compile();
}

void cApplication::CreateParticleSystem(opengl::cStaticVertexBufferObject& vbo)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fBaseSize = 0.3f;
  const float fSizeVariation = 0.1f;
  const spitfire::math::cVec3 positionVariation(3.0f, 3.0f, 3.0f);

  // NOTE: We use an extra set of texture coordinates on the end to say where the center point of that specific particle is
  opengl::cGeometryBuilder_v3_n3_t2_user3 builder(*pGeometryDataPtr);

  // Normals point out from the corners at angle slightly towards the front to very roughly approximate the normals on the front half of a sphere
  const spitfire::math::cVec3 normalTopLeft(-0.57735f, 0.57735f, 0.57735f);
  const spitfire::math::cVec3 normalTopRight(0.57735f, 0.57735f, 0.57735f);
  const spitfire::math::cVec3 normalBottomLeft(-0.57735f, -0.57735f, 0.57735f);
  const spitfire::math::cVec3 normalBottomRight(0.57735f, -0.57735f, 0.57735f);

  for (size_t i = 0; i < 100; i++) {
    const spitfire::math::cVec3 position(positionVariation * spitfire::math::cVec3(spitfire::math::randomMinusOneToPlusOnef(), spitfire::math::randomZeroToOnef(), spitfire::math::randomMinusOneToPlusOnef()));

    const float fWidth = fBaseSize + (fSizeVariation * spitfire::math::randomMinusOneToPlusOnef());
    const float fHeight = fWidth;

    const float_t fHalfWidth = fWidth * 0.5f;
    const float_t fHalfHeight = fHeight * 0.5f;
    const spitfire::math::cVec3 vMin(-fHalfWidth, -fHalfHeight, 0.0f);
    const spitfire::math::cVec3 vMax(fHalfWidth, fHalfHeight, 0.0f);

    // Add a front facing rectangle
    builder.PushBack(position + spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), normalTopRight, spitfire::math::cVec2(0.0f, 0.0f), position);
    builder.PushBack(position + spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), normalBottomLeft, spitfire::math::cVec2(1.0, 1.0), position);
    builder.PushBack(position + spitfire::math::cVec3(vMax.x, vMin.y, 0.0f), normalBottomRight, spitfire::math::cVec2(0.0f, 1.0), position);
    builder.PushBack(position + spitfire::math::cVec3(vMin.x, vMax.y, 0.0f), normalTopLeft, spitfire::math::cVec2(1.0, 0.0f), position);
    builder.PushBack(position + spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), normalBottomLeft, spitfire::math::cVec2(1.0, 1.0), position);
    builder.PushBack(position + spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), normalTopRight, spitfire::math::cVec2(0.0f, 0.0f), position);
  }

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

void cApplication::CreateTestImage(opengl::cStaticVertexBufferObject& vbo, size_t nTextureWidth, size_t nTextureHeight)
{
  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  const float fTextureWidth = float(nTextureWidth);
  const float fTextureHeight = float(nTextureHeight);

  const float fRatioWidthOverHeight = (fTextureWidth / fTextureHeight);

  const float fWidth = 1.0f;
  const float fHeight = fWidth / fRatioWidthOverHeight;

  const float_t fHalfWidth = fWidth * 0.5f;
  const float_t fHalfHeight = fHeight * 0.5f;
  const spitfire::math::cVec3 vMin(-fHalfWidth, -fHalfHeight, 0.0f);
  const spitfire::math::cVec3 vMax(fHalfWidth, fHalfHeight, 0.0f);
  const spitfire::math::cVec3 vNormal(0.0f, 0.0f, -1.0f);

  opengl::cGeometryBuilder_v3_n3_t2 builder(*pGeometryDataPtr);

  // Front facing rectangle
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), vNormal, spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMin.y, 0.0f), vNormal, spitfire::math::cVec2(0.0f, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMax.y, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, 0.0f));
  builder.PushBack(spitfire::math::cVec3(vMin.x, vMin.y, 0.0f), vNormal, spitfire::math::cVec2(fTextureWidth, fTextureHeight));
  builder.PushBack(spitfire::math::cVec3(vMax.x, vMax.y, 0.0f), vNormal, spitfire::math::cVec2(0.0f, 0.0f));

  vbo.SetData(pGeometryDataPtr);

  vbo.Compile();
}

bool cApplication::Create()
{
  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 720) || (resolution.height < 480) || ((resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8) && (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8))) {
    LOGERROR("Current screen resolution is not adequate ", resolution.width, "x", resolution.height);
    return false;
  }

  #ifdef BUILD_DEBUG
  // Override the resolution
  opengl::cSystem::GetWindowedTestResolution16By9(resolution.width, resolution.height);
  #else
  resolution.width = 1000;
  resolution.height = 562;
  #endif
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  // Set our required resolution
  pWindow = system.CreateWindow(TEXT("OpenGLmm Shaders Test"), resolution, false);
  if (pWindow == nullptr) {
    LOGERROR("Window could not be created");
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    LOGERROR("Context could not be created");
    return false;
  }

  // Create our font
  pContext->CreateFont(font, TEXT("fonts/pricedown.ttf"), 32, TEXT("shaders/font.vert"), TEXT("shaders/font.frag"));
  assert(font.IsValid());

  CreateShaders();

  // Create our text VBO
  pContext->CreateStaticVertexBufferObject(textVBO);

  pContext->CreateTextureFrameBufferObject(textureFrameBufferObjectTeapot, resolution.width, resolution.height, opengl::PIXELFORMAT::R8G8B8A8);
  assert(textureFrameBufferObjectTeapot.IsValid());

  for (size_t i = 0; i < 2; i++) {
    pContext->CreateTextureFrameBufferObjectWithDepth(textureFrameBufferObjectScreenColourAndDepth[i], resolution.width, resolution.height);
    assert(textureFrameBufferObjectScreenColourAndDepth[i].IsValid());
  }

  pContext->CreateTextureFrameBufferObjectWithDepth(textureFrameBufferObjectScreenDepth, resolution.width, resolution.height);

  pContext->CreateTexture(textureDiffuse, TEXT("textures/diffuse.png"));
  assert(textureDiffuse.IsValid());
  pContext->CreateTexture(textureFelt, TEXT("textures/felt.png"));
  assert(textureFelt.IsValid());
  pContext->CreateTexture(textureLightMap, TEXT("textures/lightmap.png"));
  assert(textureLightMap.IsValid());
  pContext->CreateTexture(textureDetail, TEXT("textures/detail.png"));
  assert(textureDetail.IsValid());

  pContext->CreateTexture(textureMetalDiffuse, TEXT("textures/metal.png"));
  assert(textureMetalDiffuse.IsValid());
  pContext->CreateTexture(textureMetalSpecular, TEXT("textures/metal_specular.jpg"));
  assert(textureMetalSpecular.IsValid());

  pContext->CreateTextureCubeMap(
    textureCubeMap,
    TEXT("textures/skybox_positive_x.jpg"),
    TEXT("textures/skybox_negative_x.jpg"),
    TEXT("textures/skybox_positive_y.jpg"),
    TEXT("textures/skybox_negative_y.jpg"),
    TEXT("textures/skybox_positive_z.jpg"),
    TEXT("textures/skybox_negative_z.jpg")
  );
  assert(textureCubeMap.IsValid());

  pContext->CreateTexture(textureMarble, TEXT("textures/marble.png"));
  assert(textureMarble.IsValid());

  pContext->CreateTexture(textureNormalMapDiffuse, TEXT("textures/floor_tile_color_map.png"));
  assert(textureNormalMapDiffuse.IsValid());

  pContext->CreateTexture(textureNormalMapSpecular, TEXT("textures/floor_tile_gloss_map.png"));
  assert(textureNormalMapSpecular.IsValid());

  pContext->CreateTexture(textureNormalMapNormal, TEXT("textures/floor_tile_normal_map.png"));
  assert(textureNormalMapNormal.IsValid());

  pContext->CreateTexture(textureNormalMapHeight, TEXT("textures/floor_tile_height_map.png"));
  assert(textureNormalMapHeight.IsValid());

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);
  CreateTeapotVBO();

  #ifdef BUILD_LARGE_STATUE_MODEL
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectStatue);
  CreateStatueVBO();
  #endif

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectScreen);
  CreateScreenRectVBO(staticVertexBufferObjectScreenRectScreen, 1.0f, 1.0f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);
  CreateGuiRectangle(staticVertexBufferObjectGuiRectangle, 1000.0f, 500.0f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectHalfScreen);
  CreateScreenHalfRectVBO(staticVertexBufferObjectScreenRectHalfScreen, 0.5f, 1.0f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectTeapot);
  CreateScreenRectVBO(staticVertexBufferObjectScreenRectTeapot, 0.25f, 0.25f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreen2DTeapot);
  CreateScreenRectVBO(staticVertexBufferObjectScreen2DTeapot, 0.25f, 0.25f, 1.0f, 1.0f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectScreenRectDebugVariableTextureSize);
  CreateScreenRectVariableTextureSizeVBO(staticVertexBufferObjectScreenRectDebugVariableTextureSize, 0.25f, 0.25f);

  pContext->CreateStaticVertexBufferObject(light.vbo);
  CreateLightBillboard();
  pContext->CreateTexture(light.texture, TEXT("textures/light.png"));
  assert(light.texture.IsValid());
  pContext->CreateTexture(textureFlare, TEXT("textures/flare.png"));
  assert(textureFlare.IsValid());

  pContext->CreateTexture(smoke.texture, TEXT("textures/particle_smoke.png"));
  assert(smoke.texture.IsValid());
  pContext->CreateStaticVertexBufferObject(smoke.vbo);
  CreateParticleSystem(smoke.vbo);

  pContext->CreateTexture(fire.texture, TEXT("textures/particle_fire.png"));
  assert(fire.texture.IsValid());
  pContext->CreateStaticVertexBufferObject(fire.vbo);
  CreateParticleSystem(fire.vbo);

  const spitfire::string_t sTestImagesPath = TEXT("textures/testimages/");
  for (spitfire::filesystem::cFolderIterator iter(sTestImagesPath); iter.IsValid(); iter.Next()) {
    if (iter.IsFile()) {
      cTextureVBOPair* pPair = new cTextureVBOPair;
      pContext->CreateTexture(pPair->texture, iter.GetFullPath());
      assert(pPair->texture.IsValid());
      pContext->CreateStaticVertexBufferObject(pPair->vbo);
      CreateTestImage(pPair->vbo, pPair->texture.GetWidth(), pPair->texture.GetHeight());
      testImages.push_back(pPair);
    }
  }

  // Create our floor
  const float fFloorSize = 100.0f;
  const float fTextureWidthWorldSpaceMeters = 1.0f;
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPlaneFloor);
  CreatePlane(staticVertexBufferObjectPlaneFloor, 1, fFloorSize, fTextureWidthWorldSpaceMeters);

  const float fRadius = 1.0f;

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPlane0);
  CreatePlane(staticVertexBufferObjectPlane0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectCube0);
  CreateCube(staticVertexBufferObjectCube0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectBox0);
  CreateBox(staticVertexBufferObjectBox0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSphere0);
  CreateSphere(staticVertexBufferObjectSphere0, 0, fRadius);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
  CreateTeapot(staticVertexBufferObjectTeapot0, 0);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectGear0);
  CreateGear(staticVertexBufferObjectGear0);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPlane2);
  CreatePlane(staticVertexBufferObjectPlane2, 2);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectCube2);
  CreateCube(staticVertexBufferObjectCube2, 2);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectBox2);
  CreateBox(staticVertexBufferObjectBox2, 2);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSphere2);
  CreateSphere(staticVertexBufferObjectSphere2, 2, fRadius);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
  CreateTeapot(staticVertexBufferObjectTeapot2, 2);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPlane3);
  CreatePlane(staticVertexBufferObjectPlane3, 3);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectCube3);
  CreateCube(staticVertexBufferObjectCube3, 3);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectBox3);
  CreateBox(staticVertexBufferObjectBox3, 3);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSphere3);
  CreateSphere(staticVertexBufferObjectSphere3, 3, fRadius);
  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
  CreateTeapot(staticVertexBufferObjectTeapot3, 3);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectPointLight);
  CreateSphere(staticVertexBufferObjectPointLight, 0, 0.3f);

  pContext->CreateStaticVertexBufferObject(staticVertexBufferObjectSpotLight);
  CreateSphere(staticVertexBufferObjectSpotLight, 0, 0.3f);

  pContext->CreateStaticVertexBufferObject(parallaxNormalMap.vbo);
  CreateNormalMappedCube();


  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Sepia"), TEXT("shaders/sepia.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Noir"), TEXT("shaders/noir.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Matrix"), TEXT("shaders/matrix.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Teal and Orange"), TEXT("shaders/tealandorange.frag")));
  simplePostRenderShaders.push_back(cSimplePostRenderShader(TEXT("Colour blind simulation"), TEXT("shaders/colourblind.frag")));


  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  ASSERT(pContext != nullptr);

  pContext->DestroyStaticVertexBufferObject(parallaxNormalMap.vbo);

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSpotLight);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPointLight);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSphere3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectBox3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectCube3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPlane3);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSphere2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectBox2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectCube2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPlane2);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectGear0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectSphere0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectBox0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectCube0);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPlane0);

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectPlaneFloor);

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectDebugVariableTextureSize);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreen2DTeapot);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectTeapot);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectHalfScreen);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectScreenRectScreen);
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);

  #ifdef BUILD_LARGE_STATUE_MODEL
  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectStatue);
  #endif

  pContext->DestroyStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);


  if (textureNormalMapHeight.IsValid()) pContext->DestroyTexture(textureNormalMapHeight);
  if (textureNormalMapNormal.IsValid()) pContext->DestroyTexture(textureNormalMapNormal);
  if (textureNormalMapSpecular.IsValid()) pContext->DestroyTexture(textureNormalMapSpecular);
  if (textureNormalMapDiffuse.IsValid()) pContext->DestroyTexture(textureNormalMapDiffuse);

  if (textureMetalSpecular.IsValid()) pContext->DestroyTexture(textureMetalSpecular);
  if (textureMetalDiffuse.IsValid()) pContext->DestroyTexture(textureMetalDiffuse);

  if (textureMarble.IsValid()) pContext->DestroyTexture(textureMarble);

  if (textureCubeMap.IsValid()) pContext->DestroyTextureCubeMap(textureCubeMap);

  if (textureDetail.IsValid()) pContext->DestroyTexture(textureDetail);
  if (textureLightMap.IsValid()) pContext->DestroyTexture(textureLightMap);
  if (textureFelt.IsValid()) pContext->DestroyTexture(textureFelt);
  if (textureDiffuse.IsValid()) pContext->DestroyTexture(textureDiffuse);

  if (textureFrameBufferObjectTeapot.IsValid()) pContext->DestroyTextureFrameBufferObject(textureFrameBufferObjectTeapot);

  if (textureFrameBufferObjectScreenDepth.IsValid()) pContext->DestroyTextureFrameBufferObject(textureFrameBufferObjectScreenDepth);

  for (size_t i = 0; i < 2; i++) {
    if (textureFrameBufferObjectScreenColourAndDepth[i].IsValid()) pContext->DestroyTextureFrameBufferObject(textureFrameBufferObjectScreenColourAndDepth[i]);
  }


  const size_t n = testImages.size();
  for (size_t i = 0; i < n; i++) {
    if (testImages[i]->texture.IsValid()) pContext->DestroyTexture(testImages[i]->texture);

    pContext->DestroyStaticVertexBufferObject(testImages[i]->vbo);

    delete testImages[i];
  }

  testImages.clear();

  // Destroy our smoke
  if (smoke.texture.IsValid()) pContext->DestroyTexture(smoke.texture);
  pContext->DestroyStaticVertexBufferObject(smoke.vbo);

  // Destroy our fire
  if (fire.texture.IsValid()) pContext->DestroyTexture(fire.texture);
  pContext->DestroyStaticVertexBufferObject(fire.vbo);

  // Destroy our light
  if (textureFlare.IsValid()) pContext->DestroyTexture(textureFlare);
  if (light.texture.IsValid()) pContext->DestroyTexture(light.texture);
  pContext->DestroyStaticVertexBufferObject(light.vbo);

  // Destroy our text VBO
  pContext->DestroyStaticVertexBufferObject(textVBO);

  // Destroy our font
  if (font.IsValid()) pContext->DestroyFont(font);

  DestroyShaders();

  shadowMapping.Destroy(*pContext);
  hdr.Destroy(*pContext);
  lensFlareDirt.Destroy(*pContext);
  dofBokeh.Destroy(*pContext);

  pContext = nullptr;

  if (pWindow != nullptr) {
    system.DestroyWindow(pWindow);
    pWindow = nullptr;
  }
}

void cApplication::CreateShaders()
{
  pContext->CreateShader(shaderColour, TEXT("shaders/colour.vert"), TEXT("shaders/colour.frag"));
  assert(shaderColour.IsCompiledProgram());

  pContext->CreateShader(shaderCubeMap, TEXT("shaders/cubemap.vert"), TEXT("shaders/cubemap.frag"));
  assert(shaderCubeMap.IsCompiledProgram());

  pContext->CreateShader(shaderBRDF, TEXT("shaders/brdf.vert"), TEXT("shaders/brdf.frag"));
  assert(shaderBRDF.IsCompiledProgram());

  pContext->CreateShader(shaderCarPaint, TEXT("shaders/carpaint.vert"), TEXT("shaders/carpaint.frag"));
  assert(shaderCarPaint.IsCompiledProgram());
  pContext->CreateShader(shaderGlass, TEXT("shaders/glass.vert"), TEXT("shaders/glass.frag"));
  assert(shaderGlass.IsCompiledProgram());

  pContext->CreateShader(shaderSilhouette, TEXT("shaders/silhouette.vert"), TEXT("shaders/silhouette.frag"));
  assert(shaderSilhouette.IsCompiledProgram());

  pContext->CreateShader(shaderCelShaded, TEXT("shaders/celshader.vert"), TEXT("shaders/celshader.frag"));
  assert(shaderCelShaded.IsCompiledProgram());

  pContext->CreateShader(light.shader, TEXT("shaders/billboard.vert"), TEXT("shaders/billboard.frag"));
  assert(light.shader.IsCompiledProgram());

  pContext->CreateShader(shaderSmoke, TEXT("shaders/smoke.vert"), TEXT("shaders/smoke.frag"));
  assert(shaderSmoke.IsCompiledProgram());

  pContext->CreateShader(shaderFire, TEXT("shaders/fire.vert"), TEXT("shaders/fire.frag"));
  assert(shaderFire.IsCompiledProgram());

  pContext->CreateShader(shaderLambert, TEXT("shaders/lambert.vert"), TEXT("shaders/lambert.frag"));
  assert(shaderLambert.IsCompiledProgram());

  pContext->CreateShader(shaderLights, TEXT("shaders/lights.vert"), TEXT("shaders/lights.frag"));
  assert(shaderLights.IsCompiledProgram());

  pContext->CreateShader(parallaxNormalMap.shader, TEXT("shaders/parallaxnormalmap.vert"), TEXT("shaders/parallaxnormalmap.frag"));
  assert(parallaxNormalMap.shader.IsCompiledProgram());

  pContext->CreateShader(shaderPassThrough, TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(shaderPassThrough.IsCompiledProgram());

  pContext->CreateShader(shaderScreen1D, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/passthrough1d.frag"));
  assert(shaderScreen1D.IsCompiledProgram());

  // TODO: Rename passthrough2d.frag and then rename passthrough2dnonrect.frag
  pContext->CreateShader(shaderScreen2D, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/passthrough2dnonrect.frag"));
  assert(shaderScreen2D.IsCompiledProgram());

  pContext->CreateShader(shaderScreenRectVariableTextureSize, TEXT("shaders/debugpassthrough2d.vert"), TEXT("shaders/passthrough2d.frag"));
  assert(shaderScreenRectVariableTextureSize.IsCompiledProgram());

  pContext->CreateShader(shaderScreenRect, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/passthrough2d.frag"));
  assert(shaderScreenRect.IsCompiledProgram());

  pContext->CreateShader(shaderScreenRectDepthShadow, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/debugshadowmaptexture2d.frag"));
  assert(shaderScreenRectDepthShadow.IsCompiledProgram());

  pContext->CreateShader(shaderScreenRectColourAndDepth, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/colouranddepth2d.frag"));
  assert(shaderScreenRectColourAndDepth.IsCompiledProgram());

  pContext->CreateShader(shaderCrate, TEXT("shaders/crate.vert"), TEXT("shaders/crate.frag"));
  assert(shaderCrate.IsCompiledProgram());

  pContext->CreateShader(shaderFog, TEXT("shaders/fog.vert"), TEXT("shaders/fog.frag"));
  assert(shaderFog.IsCompiledProgram());

  pContext->CreateShader(shaderMetal, TEXT("shaders/metal.vert"), TEXT("shaders/metal.frag"));
  assert(shaderMetal.IsCompiledProgram());
}

void cApplication::DestroyShaders()
{
  if (shaderScreenRectSimplePostRender.IsCompiledProgram()) pContext->DestroyShader(shaderScreenRectSimplePostRender);
  if (shaderScreenRectDepthShadow.IsCompiledProgram()) pContext->DestroyShader(shaderScreenRectDepthShadow);
  if (shaderScreenRectColourAndDepth.IsCompiledProgram()) pContext->DestroyShader(shaderScreenRectColourAndDepth);
  if (shaderScreenRect.IsCompiledProgram()) pContext->DestroyShader(shaderScreenRect);
  if (shaderScreenRectVariableTextureSize.IsCompiledProgram()) pContext->DestroyShader(shaderScreenRectVariableTextureSize);
  if (shaderScreen2D.IsCompiledProgram()) pContext->DestroyShader(shaderScreen2D);
  if (shaderScreen1D.IsCompiledProgram()) pContext->DestroyShader(shaderScreen1D);
  if (shaderPassThrough.IsCompiledProgram()) pContext->DestroyShader(shaderPassThrough);

  if (parallaxNormalMap.shader.IsCompiledProgram()) pContext->DestroyShader(parallaxNormalMap.shader);

  if (shaderLambert.IsCompiledProgram()) pContext->DestroyShader(shaderLambert);
  if (shaderLights.IsCompiledProgram()) pContext->DestroyShader(shaderLights);

  if (light.shader.IsCompiledProgram()) pContext->DestroyShader(light.shader);
  if (shaderSmoke.IsCompiledProgram()) pContext->DestroyShader(shaderSmoke);
  if (shaderFire.IsCompiledProgram()) pContext->DestroyShader(shaderFire);
  if (shaderCelShaded.IsCompiledProgram()) pContext->DestroyShader(shaderCelShaded);
  if (shaderSilhouette.IsCompiledProgram()) pContext->DestroyShader(shaderSilhouette);
  if (shaderGlass.IsCompiledProgram()) pContext->DestroyShader(shaderGlass);
  if (shaderBRDF.IsCompiledProgram()) pContext->DestroyShader(shaderBRDF);
  if (shaderCarPaint.IsCompiledProgram()) pContext->DestroyShader(shaderCarPaint);
  if (shaderCubeMap.IsCompiledProgram()) pContext->DestroyShader(shaderCubeMap);
  if (shaderColour.IsCompiledProgram()) pContext->DestroyShader(shaderColour);

  if (shaderMetal.IsCompiledProgram()) pContext->DestroyShader(shaderMetal);
  if (shaderFog.IsCompiledProgram()) pContext->DestroyShader(shaderFog);
  if (shaderCrate.IsCompiledProgram()) pContext->DestroyShader(shaderCrate);
}

void cApplication::RenderDebugScreenRectangleVariableSize(float x, float y, opengl::cTexture& texture)
{
  spitfire::math::cMat4 matModelView2D;
  matModelView2D.SetTranslation(x, y, 0.0f);

  pContext->BindShader(shaderScreenRectVariableTextureSize);

  pContext->SetShaderConstant("textureSize", spitfire::math::cVec2(texture.GetWidth(), texture.GetHeight()));

  pContext->BindTexture(0, texture);

  pContext->BindStaticVertexBufferObject2D(staticVertexBufferObjectScreenRectDebugVariableTextureSize);

  {
    pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

    pContext->DrawStaticVertexBufferObjectTriangles2D(staticVertexBufferObjectScreenRectDebugVariableTextureSize);
  }

  pContext->UnBindStaticVertexBufferObject2D(staticVertexBufferObjectScreenRectDebugVariableTextureSize);

  pContext->UnBindTexture(0, texture);

  pContext->UnBindShader(shaderScreenRectVariableTextureSize);
}

void cApplication::RenderScreenRectangleDepthTexture(float x, float y, opengl::cStaticVertexBufferObject& vbo, opengl::cTextureFrameBufferObject& texture, opengl::cShader& shader)
{
  spitfire::math::cMat4 matModelView2D;
  matModelView2D.SetTranslation(x, y, 0.0f);

  pContext->BindShader(shader);

  pContext->BindTextureDepthBuffer(0, texture);

  pContext->BindStaticVertexBufferObject2D(vbo);

  {
    pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

    pContext->DrawStaticVertexBufferObjectTriangles2D(vbo);
  }

  pContext->UnBindStaticVertexBufferObject2D(vbo);

  pContext->UnBindTextureDepthBuffer(0, texture);

  pContext->UnBindShader(shader);
}

void cApplication::RenderScreenRectangle(float x, float y, opengl::cStaticVertexBufferObject& vbo, opengl::cTexture& texture, opengl::cShader& shader)
{
  spitfire::math::cMat4 matModelView2D;
  matModelView2D.SetTranslation(x, y, 0.0f);

  pContext->BindShader(shader);

  pContext->BindTexture(0, texture);

  pContext->BindStaticVertexBufferObject2D(vbo);

  {
    pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

    pContext->DrawStaticVertexBufferObjectTriangles2D(vbo);
  }

  pContext->UnBindStaticVertexBufferObject2D(vbo);

  pContext->UnBindTexture(0, texture);

  pContext->UnBindShader(shader);
}

void cApplication::RenderScreenRectangle(opengl::cTexture& texture, opengl::cShader& shader)
{
  RenderScreenRectangle(texture, shader, staticVertexBufferObjectScreenRectScreen);
}

void cApplication::RenderScreenRectangle(opengl::cTexture& texture, opengl::cShader& shader, opengl::cStaticVertexBufferObject& vbo)
{
  RenderScreenRectangle(0.5f, 0.5f, vbo, texture, shader);
}

void cApplication::RenderScreenRectangleShaderAndTextureAlreadySet()
{
  RenderScreenRectangleShaderAndTextureAlreadySet(staticVertexBufferObjectScreenRectScreen);
}

void cApplication::RenderScreenRectangleShaderAndTextureAlreadySet(opengl::cStaticVertexBufferObject& vbo)
{
  spitfire::math::cMat4 matModelView2D;
  matModelView2D.SetTranslation(0.5f, 0.5f, 0.0f);

  pContext->BindStaticVertexBufferObject2D(vbo);

  {
    pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView2D);

    pContext->DrawStaticVertexBufferObjectTriangles2D(vbo);
  }

  pContext->UnBindStaticVertexBufferObject2D(vbo);
}

void cApplication::_OnWindowEvent(const opengl::cWindowEvent& event)
{
  LOG("");

  if (event.IsQuit()) {
    LOG("Quiting");
    bIsDone = true;
  }
}

void cApplication::_OnMouseEvent(const opengl::cMouseEvent& event)
{
  // These are a little too numerous to log every single one
  //LOG("");

  if (event.IsMouseMove()) {
    //LOG("Mouse move");

    if (fabs(event.GetX() - (pWindow->GetWidth() * 0.5f)) > 0.5f) {
      camera.RotateX(0.08f * (event.GetX() - (pWindow->GetWidth() * 0.5f)));
    }

    if (fabs(event.GetY() - (pWindow->GetHeight() * 0.5f)) > 1.5f) {
      camera.RotateY(0.05f * (event.GetY() - (pWindow->GetHeight() * 0.5f)));
    }
  }
}

void cApplication::_OnKeyboardEvent(const opengl::cKeyboardEvent& event)
{
  //LOG("");

  moveLightForward.Process(event.GetKeyCode(), event.IsKeyDown());
  moveLightBack.Process(event.GetKeyCode(), event.IsKeyDown());
  moveLightLeft.Process(event.GetKeyCode(), event.IsKeyDown());
  moveLightRight.Process(event.GetKeyCode(), event.IsKeyDown());

  if (event.IsKeyDown()) {
    switch (event.GetKeyCode()) {
      case SDLK_ESCAPE: {
        LOG("Escape key pressed, quiting");
        bIsDone = true;
        break;
      }
      case SDLK_w: {
        bIsMovingForward = true;
        break;
      }
      case SDLK_a: {
        bIsMovingLeft = true;
        break;
      }
      case SDLK_s: {
        bIsMovingBackward = true;
        break;
      }
      case SDLK_d: {
        bIsMovingRight = true;
        break;
      }

      case SDLK_v: {
        bIsFocalLengthDecrease = true;
        break;
      }
      case SDLK_b: {
        bIsFocalLengthIncrease = true;
        break;
      }
      case SDLK_n: {
        bIsFStopDecrease = true;
        break;
      }
      case SDLK_m: {
        bIsFStopIncrease = true;
        break;
      }

      case SDLK_SPACE: {
        //LOG("spacebar down");
        bIsRotating = false;
        break;
      }
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case SDLK_SPACE: {
        LOG("spacebar up");
        bIsRotating = true;
        break;
      }
      case SDLK_w: {
        bIsMovingForward = false;
        break;
      }
      case SDLK_a: {
        bIsMovingLeft = false;
        break;
      }
      case SDLK_s: {
        bIsMovingBackward = false;
        break;
      }
      case SDLK_d: {
        bIsMovingRight = false;
        break;
      }

      case SDLK_r: {
        bIsCameraAtLightSource = !bIsCameraAtLightSource;
        break;
      }

      case SDLK_v: {
        bIsFocalLengthDecrease = false;
        break;
      }
      case SDLK_b: {
        bIsFocalLengthIncrease = false;
        break;
      }
      case SDLK_n: {
        bIsFStopDecrease = false;
        break;
      }
      case SDLK_m: {
        bIsFStopIncrease = false;
        break;
      }
      case SDLK_c: {
        dofBokeh.SetDebugDOFShowFocus(!dofBokeh.IsDebugDOFShowFocus());
        break;
      }

      case SDLK_F5: {
        bReloadShaders = true;
        bSimplePostRenderDirty = true;
        bUpdateShaderConstants = true;
        break;
      }
      case SDLK_y: {
        simplePostRenderShaders[0].bOn = !simplePostRenderShaders[0].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_u: {
        simplePostRenderShaders[1].bOn = !simplePostRenderShaders[1].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_i: {
        simplePostRenderShaders[2].bOn = !simplePostRenderShaders[2].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_o: {
        simplePostRenderShaders[3].bOn = !simplePostRenderShaders[3].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_p: {
        simplePostRenderShaders[4].bOn = !simplePostRenderShaders[4].bOn;
        bSimplePostRenderDirty = true;
        break;
      }
      case SDLK_l: {
        if (IsColourBlindSimplePostRenderShaderEnabled()) {
          if (colourBlindMode == COLOUR_BLIND_MODE::PROTANOPIA) colourBlindMode = COLOUR_BLIND_MODE::DEUTERANOPIA;
          else if (colourBlindMode == COLOUR_BLIND_MODE::DEUTERANOPIA) colourBlindMode = COLOUR_BLIND_MODE::TRITANOPIA;
          else colourBlindMode = COLOUR_BLIND_MODE::PROTANOPIA;
          bSimplePostRenderDirty = true;
        }
        break;
      }
      case SDLK_1: {
        bIsWireframe = !bIsWireframe;
        break;
      }
      case SDLK_2: {
        bIsDirectionalLightOn = !bIsDirectionalLightOn;
        break;
      }
      case SDLK_3: {
        bIsPointLightOn = !bIsPointLightOn;
        break;
      }
      case SDLK_4: {
        bIsSpotLightOn = !bIsSpotLightOn;
        break;
      }
      case SDLK_5: {
        bIsDOFBokeh = !bIsDOFBokeh;
        break;
      }
      case SDLK_6: {
        bIsHDR = !bIsHDR;
        break;
      }
      case SDLK_7: {
        bIsLensFlareDirt = !bIsLensFlareDirt;
        break;
      }
      case SDLK_8: {
        bDebugShowFlareOnly = !bDebugShowFlareOnly;
        break;
      }
      case SDLK_9: {
        bIsSplitScreenSimplePostEffectShaders = !bIsSplitScreenSimplePostEffectShaders;
        break;
      }
    }
  }
}

std::vector<std::string> cApplication::GetInputDescription() const
{
  std::vector<std::string> description;
  description.push_back("W forward");
  description.push_back("A left");
  description.push_back("S backward");
  description.push_back("D right");
  description.push_back("Space pause rotation");
  description.push_back("F5 reload shaders");
  description.push_back("1 toggle wireframe");
  description.push_back("2 toggle directional light");
  description.push_back("3 toggle point light");
  description.push_back("4 toggle spot light");
  description.push_back("5 toggle split screen post render");
  description.push_back("Y toggle post render sepia");
  description.push_back("U toggle post render noir");
  description.push_back("I toggle post render matrix");
  description.push_back("O toggle post render teal and orange");
  description.push_back("P toggle post render colour blind simulation");
  description.push_back("L switch colour blind mode");
  description.push_back("Esc quit");

  return description;
}

size_t cApplication::GetActiveSimplePostRenderShadersCount() const
{
  size_t count = 0;

  const size_t n = simplePostRenderShaders.size();
  for (size_t i = 0; i < n; i++) {
    if (simplePostRenderShaders[i].bOn) count++;
  }

  return count;
}

bool cApplication::IsColourBlindSimplePostRenderShaderEnabled() const
{
  ASSERT(4 < simplePostRenderShaders.size());
  return simplePostRenderShaders[4].bOn;
}

size_t cApplication::GetColourBlindModeDefineValue() const
{
  if (colourBlindMode == COLOUR_BLIND_MODE::PROTANOPIA) return 0;
  else if (colourBlindMode == COLOUR_BLIND_MODE::DEUTERANOPIA) return 1;

  return 2;
}

void SaveFBO(const opengl::cTextureFrameBufferObject& fbo, const spitfire::string_t& sFilePath)
{
  const size_t nCount = fbo.GetWidth() * fbo.GetHeight() * 4;
  float* data = new float[nCount];
  const GLenum textureType = fbo.GetTextureType();
  glBindTexture(textureType, fbo.GetTexture());
  glGetTexImage(textureType, 0, GL_RGBA, GL_FLOAT, data);
  glBindTexture(textureType, 0);

  uint8_t* dataBytes = new uint8_t[nCount];
  for (size_t i = 0; i < nCount; i += 4) {
    dataBytes[i] = uint8_t(255.0f * data[i]);
    dataBytes[i + 1] = uint8_t(255.0f * data[i + 1]);
    dataBytes[i + 2] = uint8_t(255.0f * data[i + 2]);
    dataBytes[i + 3] = 255;
  }

  delete[] data;
  data = nullptr;

  voodoo::cImage image;
  image.CreateFromBuffer(dataBytes, fbo.GetWidth(), fbo.GetHeight(), voodoo::PIXELFORMAT::R8G8B8A8);
  delete[] dataBytes;
  dataBytes = nullptr;

  image.SaveToBMP(sFilePath);
}

void cApplication::Run()
{
  LOG("");

  assert(pContext != nullptr);
  assert(pContext->IsValid());
  assert(font.IsValid());
  assert(textureDiffuse.IsValid());
  assert(textureFelt.IsValid());
  assert(textureLightMap.IsValid());
  assert(textureDetail.IsValid());
  assert(textureCubeMap.IsValid());
  assert(textureMarble.IsValid());
  assert(textureMetalDiffuse.IsValid());
  assert(textureMetalSpecular.IsValid());
  assert(textureNormalMapDiffuse.IsValid());
  assert(textureNormalMapSpecular.IsValid());
  assert(textureNormalMapHeight.IsValid());
  assert(textureNormalMapNormal.IsValid());
  assert(shaderColour.IsCompiledProgram());
  assert(shaderCubeMap.IsCompiledProgram());
  assert(shaderBRDF.IsCompiledProgram());
  assert(shaderCarPaint.IsCompiledProgram());
  assert(shaderGlass.IsCompiledProgram());
  assert(shaderSilhouette.IsCompiledProgram());
  assert(shaderCelShaded.IsCompiledProgram());
  assert(light.texture.IsValid());
  assert(light.shader.IsCompiledProgram());
  assert(textureFlare.IsValid());
  assert(shaderSmoke.IsCompiledProgram());
  assert(shaderFire.IsCompiledProgram());
  assert(shaderLambert.IsCompiledProgram());
  assert(shaderLights.IsCompiledProgram());
  assert(parallaxNormalMap.shader.IsCompiledProgram());
  assert(shaderPassThrough.IsCompiledProgram());
  assert(shaderScreen1D.IsCompiledProgram());
  assert(shaderScreen2D.IsCompiledProgram());
  assert(shaderScreenRectVariableTextureSize.IsCompiledProgram());
  assert(shaderScreenRect.IsCompiledProgram());
  assert(shaderScreenRectDepthShadow.IsCompiledProgram());
  assert(shaderScreenRectColourAndDepth.IsCompiledProgram());

  assert(staticVertexBufferObjectLargeTeapot.IsCompiled());
  #ifdef BUILD_LARGE_STATUE_MODEL
  assert(staticVertexBufferObjectStatue.IsCompiled());
  #endif
  assert(staticVertexBufferObjectScreenRectScreen.IsCompiled());
  assert(staticVertexBufferObjectScreenRectHalfScreen.IsCompiled());
  assert(staticVertexBufferObjectScreenRectDebugVariableTextureSize.IsCompiled());
  assert(staticVertexBufferObjectScreenRectTeapot.IsCompiled());
  assert(staticVertexBufferObjectScreen2DTeapot.IsCompiled());


  assert(shaderCrate.IsCompiledProgram());
  assert(shaderFog.IsCompiledProgram());
  assert(shaderMetal.IsCompiledProgram());

  assert(staticVertexBufferObjectPlaneFloor.IsCompiled());

  assert(staticVertexBufferObjectPlane0.IsCompiled());
  assert(staticVertexBufferObjectCube0.IsCompiled());
  assert(staticVertexBufferObjectBox0.IsCompiled());
  assert(staticVertexBufferObjectSphere0.IsCompiled());
  assert(staticVertexBufferObjectTeapot0.IsCompiled());
  //assert(staticVertexBufferObjectGear0.IsCompiled());

  assert(staticVertexBufferObjectPlane2.IsCompiled());
  assert(staticVertexBufferObjectCube2.IsCompiled());
  assert(staticVertexBufferObjectBox2.IsCompiled());
  assert(staticVertexBufferObjectSphere2.IsCompiled());
  assert(staticVertexBufferObjectTeapot2.IsCompiled());

  assert(staticVertexBufferObjectPlane3.IsCompiled());
  assert(staticVertexBufferObjectCube3.IsCompiled());
  assert(staticVertexBufferObjectBox3.IsCompiled());
  assert(staticVertexBufferObjectSphere3.IsCompiled());
  assert(staticVertexBufferObjectTeapot3.IsCompiled());

  assert(staticVertexBufferObjectPointLight.IsCompiled());
  assert(staticVertexBufferObjectSpotLight.IsCompiled());

  assert(light.texture.IsValid());
  assert(light.vbo.IsCompiled());

  assert(smoke.texture.IsValid());
  assert(smoke.vbo.IsCompiled());

  assert(fire.texture.IsValid());
  assert(fire.vbo.IsCompiled());

  assert(parallaxNormalMap.vbo.IsCompiled());

  dofBokeh.Init(*pContext);
  dofBokeh.Resize(*this, *pContext);

  hdr.Init(*pContext);
  hdr.Resize(*this, *pContext);

  lensFlareDirt.Init(*this, *pContext);
  lensFlareDirt.Resize(*pContext);

  shadowMapping.Init(*pContext);

  // Print the input instructions
  const std::vector<std::string> inputDescription = GetInputDescription();
  const size_t n = inputDescription.size();
  for (size_t i = 0; i < n; i++) LOG(inputDescription[i]);

  // Set up the camera
  camera.SetPosition(spitfire::math::cVec3(-6.5f, 2.5f, 7.0f));
  camera.RotateX(90.0f);
  camera.RotateY(45.0f);

  // Set up the translation for our floor
  const spitfire::math::cVec3 floorPosition(0.0f, -1.0f, 0.0f);
  spitfire::math::cMat4 matTranslationFloor;
  matTranslationFloor.SetTranslation(floorPosition);

  // Set up the translations for our objects
  const size_t columns = 5; // 5 types of objects
  const size_t rows = 8; // 8 types of materials

  const float fSpacingX = 0.007f * pContext->GetWidth() / float(rows);
  const float fSpacingZ = 0.03f * pContext->GetHeight() / float(columns);
  const float fLeft = -0.5f * float(columns - 1) * fSpacingX;

  spitfire::math::cVec3 positions[columns * rows];
  size_t i = 0;
  for (size_t y = 0; y < rows; y++) {
    for (size_t x = 0; x < columns; x++) {
      positions[i].Set(fLeft + (x * fSpacingX), 0.0f, (y * fSpacingZ));
      i++;
    }
  }

  spitfire::math::cMat4 matTranslationArray[columns * rows];
  for (size_t i = 0; i < columns * rows; i++) matTranslationArray[i].SetTranslation(positions[i]);


  spitfire::math::cQuaternion rotation;
  spitfire::math::cMat4 matRotation;

  float fAngleRadians = 0.0f;
  const float fRotationSpeed = 0.001f;

  spitfire::math::cMat4 matObjectRotation;

  // Cube mapped teapot
  const spitfire::math::cVec3 positionCubeMappedTeapot(-fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationCubeMappedTeapot;
  matTranslationCubeMappedTeapot.SetTranslation(positionCubeMappedTeapot);

  // Car paint teapot
  const spitfire::math::cVec3 positionCarPaintTeapot(-3.0f * fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationCarPaintTeapot;
  matTranslationCarPaintTeapot.SetTranslation(positionCarPaintTeapot);

  // Glass teapot
  const spitfire::math::cVec3 positionGlassTeapot(-6.0f * fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationGlassTeapot;
  matTranslationGlassTeapot.SetTranslation(positionGlassTeapot);

  // Cel shaded teapot
  const spitfire::math::cVec3 positionCelShadedTeapot(-9.0f * fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationCelShadedTeapot;
  matTranslationCelShadedTeapot.SetTranslation(positionCelShadedTeapot);

  // Smoke
  const spitfire::math::cVec3 positionSmoke(-12.0f * fSpacingX, 1.5f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationSmoke;
  matTranslationSmoke.SetTranslation(positionSmoke);

  // Fire
  const spitfire::math::cVec3 positionFire(-12.0f * fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationFire;
  matTranslationFire.SetTranslation(positionFire);

  // Parallax normal mapping
  const spitfire::math::cVec3 parallaxNormalMapPosition(fSpacingX, 0.0f, (-1.0f * fSpacingZ));
  spitfire::math::cMat4 matTranslationParallaxNormalMap;
  matTranslationParallaxNormalMap.SetTranslation(parallaxNormalMapPosition);

  #ifdef BUILD_LARGE_STATUE_MODEL
  spitfire::math::cMat4 matTranslationStatue[9];
  i = 0;
  for (size_t y = 0; y < 3; y++) {
    for (size_t x = 0; x < 3; x++) {
      const spitfire::math::cVec3 position((-1.0f + float(x)) * fSpacingX, 0.0f, ((-2.0f - float(y)) * fSpacingZ));
      matTranslationStatue[i].SetTranslation(position);
      i++;
    }
  }
  #endif

  std::vector<spitfire::math::cMat4> matTranslationTestImages;
  {
    const size_t n = testImages.size();
    for (size_t i = 0; i < n; i++) {
      const spitfire::math::cVec3 position((-1.0f + float(i)) * fSpacingX, 0.0f, -5.0f * fSpacingZ);
      spitfire::math::cMat4 matTranslation;
      matTranslation.SetTranslation(position);
      matTranslationTestImages.push_back(matTranslation);
    }
  }


  spitfire::durationms_t T0 = spitfire::util::GetTimeMS();
  uint32_t Frames = 0;

  spitfire::durationms_t previousUpdateInputTime = spitfire::util::GetTimeMS();
  spitfire::durationms_t previousUpdateTime = spitfire::util::GetTimeMS();
  spitfire::durationms_t currentTime = spitfire::util::GetTimeMS();

  const float fLightLuminanceIncrease0To1 = 0.3f;

  // Green directional light
  const spitfire::math::cVec3 lightDirectionalPosition(5.0f, 5.0f, 5.0f);
  const spitfire::math::cColour lightDirectionalAmbientColour(0.2f, 0.25f, 0.2f);
  const spitfire::math::cColour lightDirectionalDiffuseColour(1.5f, 1.35f, 1.2f);
  const spitfire::math::cColour lightDirectionalSpecularColour(0.0f, 1.0f, 0.0f);

  // Red point light
  const spitfire::math::cColour lightPointColour(0.25f, 0.0f, 0.0f);
  const float lightPointAmbient = 0.15f;
  const float lightPointConstantAttenuation = 0.3f;
  const float lightPointLinearAttenuation = 0.007f;
  const float lightPointExpAttenuation = 0.00008f;

  // Blue spot light
  const spitfire::math::cVec3 lightSpotPosition(0.0f, 5.0f, 4.0f);
  const spitfire::math::cVec3 lightSpotDirection(0.0f, 1.0f, 0.0f);
  const spitfire::math::cColour lightSpotColour(0.0f, 0.0f, 1.0f);
  //const float lightSpotAmbient = 0.15f;
  //const float lightSpotConstantAttenuation = 0.3f;
  const float lightSpotLinearAttenuation = 0.002f;
  //const float lightSpotExpAttenuation = 0.00008f;
  const float fLightSpotLightConeAngle = 40.0f;
  const float lightSpotConeCosineAngle = cosf(spitfire::math::DegreesToRadians(fLightSpotLightConeAngle));

  // Material
  const spitfire::math::cColour materialAmbientColour(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour materialDiffuseColour(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour materialSpecularColour(1.0f, 1.0f, 1.0f);
  const float fMaterialShininess = 25.0f;

  // Use Cornflower blue as the sky colour
  // http://en.wikipedia.org/wiki/Cornflower_blue
  const spitfire::math::cColour cornFlowerBlue(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f);

  const spitfire::math::cColour skyColour = cornFlowerBlue;

  const float fFogStart = 5.0f;
  const float fFogEnd = 15.0f;
  //const float fFogDensity = 0.5f;


  const uint32_t uiUpdateInputDelta = uint32_t(1000.0f / 120.0f);
  const uint32_t uiUpdateDelta = uint32_t(1000.0f / 60.0f);

  while (!bIsDone) {
    // Update state
    currentTime = spitfire::util::GetTimeMS();

    if ((currentTime - previousUpdateInputTime) > uiUpdateInputDelta) {
      // Update window events
      pWindow->ProcessEvents();

      if (pWindow->IsActive()) {
        // Hide the cursor
        pWindow->ShowCursor(false);

        // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
        pWindow->WarpCursorToMiddleOfScreen();
      } else {
        // Show the cursor
        pWindow->ShowCursor(true);
      }

      previousUpdateInputTime = currentTime;
    }

    if ((currentTime - previousUpdateTime) > uiUpdateDelta) {
      // Update the camera
      const float fDistance = 0.1f;
      if (bIsMovingForward) camera.MoveZ(fDistance);
      if (bIsMovingBackward) camera.MoveZ(-fDistance);
      if (bIsMovingLeft) camera.MoveX(-fDistance);
      if (bIsMovingRight) camera.MoveX(fDistance);

      // Update the light position
      if (moveLightForward.bDown) lightPointPosition.x += fDistance;
      if (moveLightBack.bDown) lightPointPosition.x -= fDistance;
      if (moveLightLeft.bDown) lightPointPosition.z -= fDistance;
      if (moveLightRight.bDown) lightPointPosition.z += fDistance;

      // Update object rotation
      if (bIsRotating) fAngleRadians += float(uiUpdateDelta) * fRotationSpeed;

      rotation.SetFromAxisAngle(spitfire::math::v3Up, fAngleRadians);

      matObjectRotation.SetRotation(rotation);

      if (bIsFocalLengthIncrease) dofBokeh.IncreaseFocalLength();
      if (bIsFocalLengthDecrease) dofBokeh.DecreaseFocalLength();

      if (bIsFStopIncrease) dofBokeh.IncreaseFStop();
      if (bIsFStopDecrease) dofBokeh.DecreaseFStop();

      previousUpdateTime = currentTime;
    }

    if (bReloadShaders) {
      LOG("Reloading shaders");

      // Destroy and create our shaders
      DestroyShaders();
      CreateShaders();

      bReloadShaders = false;
    }

    if (bUpdateShaderConstants) {
      // Set our shader constants
      pContext->BindShader(shaderFog);
        pContext->SetShaderConstant("fog.colour", skyColour);
        pContext->SetShaderConstant("fog.fStart", fFogStart);
        pContext->SetShaderConstant("fog.fEnd", fFogEnd);
        //pContext->SetShaderConstant("fog.fDensity", fFogDensity);
      pContext->UnBindShader(shaderFog);

      pContext->BindShader(shaderLights);
        // Directional light
        pContext->SetShaderConstant("lightDirectional.ambientColour", lightDirectionalAmbientColour);
        pContext->SetShaderConstant("lightDirectional.diffuseColour", lightDirectionalDiffuseColour);
        pContext->SetShaderConstant("lightDirectional.specularColour", lightDirectionalSpecularColour);

        // Point light
        pContext->SetShaderConstant("lightPointLight.colour", lightPointColour);
        pContext->SetShaderConstant("lightPointLight.fAmbient", lightPointAmbient);
        pContext->SetShaderConstant("lightPointLight.fConstantAttenuation", lightPointConstantAttenuation);
        pContext->SetShaderConstant("lightPointLight.fLinearAttenuation", lightPointLinearAttenuation);
        pContext->SetShaderConstant("lightPointLight.fExpAttenuation", lightPointExpAttenuation);

        // Spot light
        pContext->SetShaderConstant("lightSpotLight.colour", lightSpotColour);
        //pContext->SetShaderConstant("lightSpotLight.fAmbient", lightSpotAmbient);
        //pContext->SetShaderConstant("lightSpotLight.fConstantAttenuation", lightSpotConstantAttenuation);
        pContext->SetShaderConstant("lightSpotLight.fLinearAttenuation", lightSpotLinearAttenuation);
        //pContext->SetShaderConstant("lightSpotLight.fExpAttenuation", lightSpotExpAttenuation);
        pContext->SetShaderConstant("lightSpotLight.fConeCosineAngle", lightSpotConeCosineAngle);

        // Setup materials
        pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
        pContext->SetShaderConstant("material.diffuseColour", materialDiffuseColour);
        pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
        pContext->SetShaderConstant("material.fShininess", fMaterialShininess);
      pContext->UnBindShader(shaderLights);

      pContext->BindShader(parallaxNormalMap.shader);
        // Set up scale bias
        pContext->SetShaderConstant("ParallaxScaleBias", spitfire::math::cVec2(0.01f, 0.01f));

        // Directional light
        pContext->SetShaderConstant("directionalLight.ambientColour", lightDirectionalAmbientColour);
        pContext->SetShaderConstant("directionalLight.diffuseColour", lightDirectionalDiffuseColour);
        pContext->SetShaderConstant("directionalLight.specularColour", lightDirectionalSpecularColour);

        // Setup materials
        pContext->SetShaderConstant("material.ambientColour", materialAmbientColour);
        pContext->SetShaderConstant("material.diffuseColour", materialDiffuseColour);
        pContext->SetShaderConstant("material.specularColour", materialSpecularColour);
        pContext->SetShaderConstant("material.fShininess", fMaterialShininess);
      pContext->UnBindShader(parallaxNormalMap.shader);

      pContext->BindShader(shaderFire);
        pContext->SetShaderConstant("diffuseColour", spitfire::math::cColour3(1.0f, 0.3f, 0.0f));
      pContext->UnBindShader(shaderFire);

      bUpdateShaderConstants = false;
    }


    // Update our text
    CreateText();
    assert(textVBO.IsCompiled());

    const spitfire::math::cMat4 matProjection = pContext->CalculateProjectionMatrix();

    const spitfire::math::cMat4 matView = (bIsCameraAtLightSource ? shadowMapping.GetView() : camera.CalculateViewMatrix());
    const spitfire::math::cVec3 lightDirection(0.1f, -1.0f, 0.0f);

    // Set up the lights shader
    pContext->BindShader(shaderLights);
      pContext->SetShaderConstant("matView", matView);

      // Directional light
      pContext->SetShaderConstant("lightDirectional.bOn", bIsDirectionalLightOn ? 1 : 0);
      pContext->SetShaderConstant("lightDirectional.direction", matView * -lightDirection);

      // Point light
      pContext->SetShaderConstant("lightPointLight.bOn", bIsPointLightOn ? 1 : 0);
      pContext->SetShaderConstant("lightPointLight.position", lightPointPosition);

      // Spot light
      pContext->SetShaderConstant("lightSpotLight.bOn", bIsSpotLightOn ? 1 : 0);
      pContext->SetShaderConstant("lightSpotLight.position", matView * lightSpotPosition);
      pContext->SetShaderConstant("lightSpotLight.direction", matView * lightSpotDirection);
    pContext->UnBindShader(shaderLights);

    // Set up the cube map shader
    pContext->BindShader(shaderCubeMap);
      //pContext->SetShaderConstant("matView", matView);
      pContext->SetShaderConstant("cameraPos", camera.GetPosition());
    pContext->UnBindShader(shaderCubeMap);

    if (bSimplePostRenderDirty) {
      // Destroy any existing shader
      if (shaderScreenRectSimplePostRender.IsCompiledProgram()) pContext->DestroyShader(shaderScreenRectSimplePostRender);

      if (GetActiveSimplePostRenderShadersCount() != 0) {
        std::map<std::string, int> mapDefinesToAdd;

        // Load the vertex shader
        std::string sVertexShaderText;
        spitfire::storage::ReadText(TEXT("shaders/passthrough2d.vert"), sVertexShaderText);

        // Create our fragment shader
        std::string sFragmentShaderText =
          "#version 330\n"
          "\n"
        ;

        // Add the colour blind define that says which colour blind mode we are using
        if (IsColourBlindSimplePostRenderShaderEnabled()) {
          const int iColourBlindMode = int(GetColourBlindModeDefineValue());
          mapDefinesToAdd["COLORBLIND_MODE"] = iColourBlindMode;
        }

        sFragmentShaderText +=
          "uniform sampler2DRect texUnit0; // Diffuse texture\n"
          "\n"
          "smooth in vec2 vertOutTexCoord;\n"
          "\n"
          "out vec4 fragmentColour;\n"
          "\n"
        ;

        const size_t n = simplePostRenderShaders.size();
        for (size_t i = 0; i < n; i++) {
          if (simplePostRenderShaders[i].bOn) {
            // Load the shader text
            std::string sTempFragmentShaderText;
            spitfire::storage::ReadText(simplePostRenderShaders[i].sFragmentShaderFilePath, sTempFragmentShaderText);

            // Skip the version
            if (spitfire::string::StartsWith(sTempFragmentShaderText, "#version")) {
              size_t index = 0;
              if (spitfire::string::Find(sTempFragmentShaderText, "\n", index)) sTempFragmentShaderText = sTempFragmentShaderText.substr(index);
            }

            // Rename the ApplyFilter function to ProcessFilter0..n
            // NOTE: We use ProcessFilter0..n instead of ApplyFilter0..n to avoid an endless loop in spitfire::string::Replace
            const std::string sFixedFragmentShaderText =
              "// " + spitfire::string::ToUTF8(simplePostRenderShaders[i].sName) +
              spitfire::string::Replace(sTempFragmentShaderText, "ApplyFilter", "ProcessFilter" + spitfire::string::ToUTF8(spitfire::string::ToString(i))) +
              "\n"
            ;

            // Add it to the existing fragment shader
            sFragmentShaderText += sFixedFragmentShaderText;
          }
        }


        // Add our main function that calls the ApplyShader0..n functions
        sFragmentShaderText +=
          "\n"
          "void main()\n"
          "{\n"
          "  vec4 colour = texture(texUnit0, vertOutTexCoord);\n"
          "\n"
        ;

        for (size_t i = 0; i < n; i++) {
          if (simplePostRenderShaders[i].bOn) sFragmentShaderText += "  colour = ProcessFilter" + spitfire::string::ToUTF8(spitfire::string::ToString(i)) + "(colour);\n";
        }

        sFragmentShaderText +=
          "\n"
          "  fragmentColour = colour;\n"
          "}\n"
          "\n"
        ;

        // Create the shader
        pContext->CreateShaderFromText(shaderScreenRectSimplePostRender, sVertexShaderText, sFragmentShaderText, TEXT("shaders/"), mapDefinesToAdd);
      }

      bSimplePostRenderDirty = false;
    }

    {
      // Render a few items from the scene into the frame buffer object for use later
      const spitfire::math::cColour clearColour(0.0f, 1.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(textureFrameBufferObjectTeapot);

      if (bIsWireframe) pContext->EnableWireframe();

      pContext->BindShader(shaderCubeMap);

      pContext->BindTextureCubeMap(0, textureCubeMap);

      pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      {
        pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationCubeMappedTeapot * matObjectRotation);

        pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
      }

      pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      pContext->UnBindTextureCubeMap(0, textureCubeMap);

      pContext->UnBindShader(shaderCubeMap);

      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToTexture(textureFrameBufferObjectTeapot);
    }

    {
      // Render our shadow map
      shadowMapping.BeginRenderToShadowMap(*pContext, lightPointPosition, lightDirection);

      // Render occluding objects
      const int half = 6;
      const int interval = 5;
      for (int z = -(half); z < half; z += interval) {
        for (int y = 1; y < half; y += interval) {
          for (int x = -(half); x < half; x += interval) {
            const spitfire::math::cVec3 position = spitfire::math::cVec3(float(x), float(y), float(z));
            spitfire::math::cMat4 matTranslation;
            matTranslation.SetTranslation(position);
            spitfire::math::cMat4 matRotation;
            matRotation.SetRotation(-rotation);
            const spitfire::math::cMat4 matModel = matTranslation * matRotation;
            shadowMapping.RenderObjectToShadowMapSetMatrices(*pContext, matModel);

            opengl::cStaticVertexBufferObject& vbo = staticVertexBufferObjectLargeTeapot;
            pContext->BindStaticVertexBufferObject(vbo);
            pContext->DrawStaticVertexBufferObjectTriangles(vbo);
            pContext->UnBindStaticVertexBufferObject(vbo);
          }
        }
      }

      shadowMapping.EndRenderToShadowMap(*pContext);
    }

    size_t outputFBO = 0;
    size_t inputFBO = 1;

    {
      opengl::cTextureFrameBufferObject& fbo = textureFrameBufferObjectScreenColourAndDepth[outputFBO];

      // Render the scene into a texture for later
      pContext->SetClearColour(skyColour);

      pContext->BeginRenderToTexture(fbo);

      if (bIsWireframe) pContext->EnableWireframe();


      pContext->BindShader(shaderCubeMap);

      // Render the cube mapped teapot
      pContext->BindTextureCubeMap(0, textureCubeMap);

      pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      {
        pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationCubeMappedTeapot * matObjectRotation);

        pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
      }

      pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

      pContext->UnBindTextureCubeMap(0, textureCubeMap);

      pContext->UnBindShader(shaderCubeMap);


      {
        pContext->BindShader(shaderCarPaint);

        // Render the car paint teapot
        pContext->BindTexture(0, textureDiffuse);
        pContext->BindTextureCubeMap(1, textureCubeMap);

        // Set our constants
        const spitfire::math::cMat4 matModelView = matView * matTranslationCarPaintTeapot;
        pContext->SetShaderConstant("fvLightPosition", matModelView * lightDirectionalPosition);
        pContext->SetShaderConstant("fvEyePosition", spitfire::math::cVec3());// matModelView * camera.GetPosition());

        pContext->SetShaderConstant("cameraPos", camera.GetPosition());
        pContext->SetShaderConstant("matModel", matTranslationCarPaintTeapot * matObjectRotation);

        // The world matrix is the model matrix apparently, matObjectToBeRendered
        //const spitfire::math::cMat4 matWorld = matTranslationCarPaintTeapot;
        //const spitfire::math::cMat4 matWorldInverseTranspose = matWorld.GetInverseTranspose();
        //pContext->SetShaderConstant("matWorldInverseTranspose", matWorldInverseTranspose);

        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

        {
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCarPaintTeapot * matObjectRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
        }

        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

        pContext->UnBindTextureCubeMap(1, textureCubeMap);
        pContext->UnBindTexture(0, textureDiffuse);

        pContext->UnBindShader(shaderCarPaint);
      }

      {
        pContext->BindShader(shaderGlass);

        // Render the glass teapot
        pContext->BindTexture(0, textureMetalDiffuse);
        pContext->BindTexture(1, textureMetalSpecular);
        pContext->BindTextureCubeMap(2, textureCubeMap);

        // Set our constants
        pContext->SetShaderConstant("cameraPosition", camera.GetPosition());

        const float IoR_R = 1.14f;
        const float IoR_G = 1.12f;
        const float IoR_B = 1.10f;
        pContext->SetShaderConstant("IoR_Values", spitfire::math::cColour3(IoR_R, IoR_G, IoR_B));

        const float fresnelR = 0.15f;
        const float fresnelG = 2.0f;
        const float fresnelB = 0.0f;
        pContext->SetShaderConstant("fresnelValues", spitfire::math::cColour3(fresnelR, fresnelG, fresnelB));

        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

        {
          pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationGlassTeapot * matObjectRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
        }

        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

        pContext->UnBindTextureCubeMap(2, textureCubeMap);
        pContext->UnBindTexture(1, textureMetalSpecular);
        pContext->UnBindTexture(0, textureMetalDiffuse);

        pContext->UnBindShader(shaderGlass);
      }

      {
        // Render the cel shaded teapot which consists of a black silhouette pass and a cel shaded pass

        // Black silhouette
        {
          if (bIsWireframe) pContext->DisableWireframe();

          glCullFace(GL_FRONT); // enable culling of front faces

          pContext->BindShader(shaderSilhouette);

          // Set our constants
          const float fOffset = 0.25f;
          pContext->SetShaderConstant("fOffset", fOffset);

          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          {
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationCelShadedTeapot * matObjectRotation);

            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
          }

          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          pContext->UnBindShader(shaderSilhouette);

          glCullFace(GL_BACK); // Reset culling to back faces

          if (bIsWireframe) pContext->EnableWireframe();
        }

        // Cel shaded
        {
          pContext->BindShader(shaderCelShaded);

          // Set our constants
          pContext->SetShaderConstant("cameraPosition", matView * camera.GetPosition());
          pContext->SetShaderConstant("lightPosition", matView * lightPointPosition);
          pContext->SetShaderConstant("colour", spitfire::math::cVec3(0.0f, 0.75f, 0.75f));
          pContext->SetShaderConstant("ambientMaterial", spitfire::math::cVec3(0.04f, 0.04f, 0.04f));
          pContext->SetShaderConstant("specularMaterial", spitfire::math::cVec3(0.5f, 0.5f, 0.5f));
          pContext->SetShaderConstant("fShininess", 50.0f);

          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          {
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationCelShadedTeapot * matObjectRotation);

            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
          }

          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

          pContext->UnBindShader(shaderCelShaded);
        }
      }


#ifdef BUILD_LARGE_STATUE_MODEL
      // Render the statues
      {
        pContext->BindShader(shaderLights);

        pContext->BindTexture(0, textureMarble);

        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectStatue);

        for (size_t i = 0; i < 9; i++) {
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationStatue[i] * matObjectRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectStatue);
        }

        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectStatue);

        pContext->UnBindTexture(0, textureMarble);

        pContext->UnBindShader(shaderLights);
      }
#endif


      // Render the parallax normal map cube
      {
        pContext->BindShader(parallaxNormalMap.shader);

        pContext->BindTexture(0, textureNormalMapDiffuse);
        pContext->BindTexture(1, textureNormalMapSpecular);
        pContext->BindTexture(2, textureNormalMapNormal);
        pContext->BindTexture(3, textureNormalMapHeight);

        // Set up the shader uniforms
        pContext->SetShaderConstant("directionalLight.direction", (parallaxNormalMapPosition - lightDirectionalPosition).GetNormalised());

        pContext->BindStaticVertexBufferObject(parallaxNormalMap.vbo);

        pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationParallaxNormalMap * matObjectRotation);

        pContext->DrawStaticVertexBufferObjectTriangles(parallaxNormalMap.vbo);

        pContext->UnBindStaticVertexBufferObject(parallaxNormalMap.vbo);

        pContext->UnBindTexture(3, textureNormalMapHeight);
        pContext->UnBindTexture(2, textureNormalMapNormal);
        pContext->UnBindTexture(1, textureNormalMapSpecular);
        pContext->UnBindTexture(0, textureNormalMapDiffuse);

        pContext->UnBindShader(parallaxNormalMap.shader);
      }


      // Render the lights
      {
        pContext->BindShader(shaderColour);

        {
          pContext->SetShaderConstant("colour", lightDirectionalDiffuseColour);

          spitfire::math::cMat4 matTransform;
          matTransform.SetTranslation(lightDirectionalPosition);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTransform);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
        }

        if (!bIsCameraAtLightSource) {
          pContext->SetShaderConstant("colour", util::ChangeLuminance(lightPointColour, fLightLuminanceIncrease0To1));

          spitfire::math::cMat4 matTransform;
          matTransform.SetTranslation(lightPointPosition);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTransform);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
        }

        {
          pContext->SetShaderConstant("colour", util::ChangeLuminance(lightSpotColour, fLightLuminanceIncrease0To1));

          spitfire::math::cMat4 matTransform;
          matTransform.SetTranslation(lightSpotPosition);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTransform);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
        }

        pContext->UnBindShader(shaderColour);
      }


      // Render the metal objects

      {
        pContext->BindShader(shaderMetal);

        pContext->BindTexture(0, textureMetalDiffuse);
        pContext->BindTexture(1, textureMetalSpecular);
        pContext->BindTextureCubeMap(2, textureCubeMap);

        // Set our constants
        pContext->SetShaderConstant("cameraPosition", camera.GetPosition());

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[0] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube2);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[1] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox2);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[2] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[3] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[4] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
        }

        pContext->UnBindTextureCubeMap(2, textureCubeMap);
        pContext->UnBindTexture(1, textureMetalSpecular);
        pContext->UnBindTexture(0, textureMetalDiffuse);

        pContext->UnBindShader(shaderMetal);
      }




      // Render shadow mapped objects
      {
        pContext->BindShader(shadowMapping.GetShadowMapShader());

        pContext->BindTexture(0, textureFelt);
        pContext->BindTextureDepthBuffer(1, shadowMapping.GetShadowMapTexture());

        //pContext->SetShaderConstant("LightPosition_worldspace", lightPointPosition);

        // Render the ground
        const spitfire::math::cMat4& matModel = matTranslationFloor;
        pContext->SetShaderConstant("DepthBiasMVP", shadowMapping.GetDepthBiasMVP(matModel));
        pContext->SetShaderConstant("LightInvDirection_worldspace", matModel * shadowMapping.GetLightInvDirection());
        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlaneFloor);
        pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matModel);
        pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlaneFloor);
        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlaneFloor);

        const int half = 6;
        const int interval = 5;
        for (int z = -(half); z < half; z += interval) {
          for (int y = 1; y < half; y += interval) {
            for (int x = -(half); x < half; x += interval) {
              const spitfire::math::cVec3 position = spitfire::math::cVec3(float(x), float(y), float(z));

              spitfire::math::cMat4 matObjectTranslation;
              matObjectTranslation.SetTranslation(position);

              spitfire::math::cMat4 matObjectRotation;
              matObjectRotation.SetRotation(-rotation);

              const spitfire::math::cMat4 matModel = matObjectTranslation * matObjectRotation;
              pContext->SetShaderConstant("DepthBiasMVP", shadowMapping.GetDepthBiasMVP(matModel));
              pContext->SetShaderConstant("LightInvDirection_worldspace", matModel * shadowMapping.GetLightInvDirection());

              opengl::cStaticVertexBufferObject& vbo = staticVertexBufferObjectLargeTeapot;
              pContext->BindStaticVertexBufferObject(vbo);
              pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matModel);
              pContext->DrawStaticVertexBufferObjectTriangles(vbo);
              pContext->UnBindStaticVertexBufferObject(vbo);
            }
          }
        }

        pContext->UnBindTextureDepthBuffer(1, shadowMapping.GetShadowMapTexture());
        pContext->UnBindTexture(0, textureFelt);

        pContext->UnBindShader(shadowMapping.GetShadowMapShader());
      }


      // Render the textured objects
      pContext->BindShader(shaderCrate);

      pContext->BindTexture(0, textureDiffuse);
      pContext->BindTexture(1, textureLightMap);
      pContext->BindTexture(2, textureDetail);

      {
        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[5] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[6] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[7] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[8] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere3);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[9] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
        }
      }

      pContext->UnBindTexture(2, textureDetail);
      pContext->UnBindTexture(1, textureLightMap);
      pContext->UnBindTexture(0, textureDiffuse);

      pContext->UnBindShader(shaderCrate);


      // Render the foggy objects
      pContext->BindShader(shaderFog);

      pContext->BindTexture(0, textureDiffuse);
      pContext->BindTexture(1, textureDetail);

      {
        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[10] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[11] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[12] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[13] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[14] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
        }
      }

      pContext->UnBindTexture(1, textureDetail);
      pContext->UnBindTexture(0, textureDiffuse);

      pContext->UnBindShader(shaderFog);


      // Render the cube mapped objects
      pContext->BindShader(shaderCubeMap);

      pContext->BindTextureCubeMap(0, textureCubeMap);

      {
        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[15] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube2);
            pContext->SetShaderConstant("matModel", matTranslationArray[16] * matObjectRotation);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[16] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox2);
            pContext->SetShaderConstant("matModel", matTranslationArray[17] * matObjectRotation);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[17] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
            pContext->SetShaderConstant("matModel", matTranslationArray[18] * matObjectRotation);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[18] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere2);
        }

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
            pContext->SetShaderConstant("matModel", matTranslationArray[19] * matObjectRotation);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationArray[19] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot2);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot2);
        }
      }

      pContext->UnBindTextureCubeMap(0, textureCubeMap);

      pContext->UnBindShader(shaderCubeMap);



      // Render the lambert shaded objects
      pContext->BindShader(shaderLambert);

      {
        pContext->SetShaderConstant("lightPosition", lightDirectionalPosition);
        pContext->SetShaderConstant("matView", matView);

        // Mirror's Edge style lighting (But much uglier because this is just a cheap knock off)
        pContext->SetShaderConstant("ambientColour", spitfire::math::cColour3(0.0f, 0.35f, 1.0f));
        pContext->SetShaderConstant("lightColour", spitfire::math::cColour3(1.0f, 1.0f, 0.83f));

        const spitfire::math::cColour3 white(0.9f, 0.9f, 0.9f);
        const spitfire::math::cColour3 red(1.0f, 0.0f, 0.0f);

        {
          pContext->SetShaderConstant("colour", white);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectPlane0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[20] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectPlane0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectPlane0);
        }

        {
          pContext->SetShaderConstant("colour", white);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectCube0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[21] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectCube0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectCube0);
        }

        {
          pContext->SetShaderConstant("colour", red);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectBox0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[22] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectBox0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectBox0);
        }

        {
          pContext->SetShaderConstant("colour", white);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[23] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere0);
        }

        {
          pContext->SetShaderConstant("colour", red);
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
            pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[24] * matObjectRotation);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot0);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot0);
        }
      }

      pContext->UnBindShader(shaderLambert);


      // Render spheres with a variety of colours
      {
        pContext->BindShader(shaderColour);

        const spitfire::math::cColour3 colours[columns] = {
          spitfire::math::cColour3(7.0f / 255.0f, 16.0f / 255.0f, 141.0f / 255.0f), // Dark blue
          spitfire::math::cColour3(122.0f / 255.0f, 143.0f / 255.0f, 248.0f / 255.0f), // Light blue
          spitfire::math::cColour3(3.0f * 122.0f / 255.0f, 3.0f * 143.0f / 255.0f, 3.0f * 248.0f / 255.0f), // Super bright blue
          spitfire::math::cColour3(0.8f, 0.498039f, 0.196078f), // Gold
          spitfire::math::cColour3(3 * 0.8f, 3 * 0.498039f, 3 * 0.196078f), // Bright gold
        };
        const spitfire::math::cColour3 colours2[columns] = {
          spitfire::math::cColour3(0.0f, 0.0f, 0.0f), // Black
          spitfire::math::cColour3(0.5f, 0.5f, 0.5f), // Grey
          spitfire::math::cColour3(1.0f, 1.0f, 1.0f), // White
          spitfire::math::cColour3(2.0f, 2.0f, 2.0f), // Bright white
          spitfire::math::cColour3(3.0f, 3.0f, 3.0f), // Super bright white
        };

        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectSphere0);

        // First column
        for (size_t index = 0; index < 5; index++) {
          pContext->SetShaderConstant("colour", spitfire::math::cColour(colours[index].r, colours[index].g, colours[index].b, 1.0f));

          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[(5 * columns) + index]);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
        }

        // Second column
        for (size_t index = 0; index < 5; index++) {
          pContext->SetShaderConstant("colour", spitfire::math::cColour(colours2[index].r, colours2[index].g, colours2[index].b, 1.0f));

          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationArray[(6 * columns) + index]);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectSphere0);
        }

        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectSphere0);

        pContext->UnBindShader(shaderColour);
      }



      opengl::cSystem::GetErrorString();

      {
        // Render BRDF objects
        pContext->BindShader(shaderBRDF);

        pContext->BindTexture(0, textureDiffuse);

        // Set our constants

        // Start with 2^6 = 64 samples
        const GLuint g_m = 6;

        // Roughness of the material
        const GLfloat roughness[columns] = {
          0.1f,
          0.3f,
          0.5f,
          0.7f,
          1.0f
        };

        // Reflection coefficient see http://en.wikipedia.org/wiki/Schlick%27s_approximation
        const GLfloat R0[columns] = {
          1.0f,
          0.8f,
          0.6f,
          0.4f,
          0.2f
        };

        const spitfire::math::cColour3 colours[columns] = {
          spitfire::math::cColour3(0.2f, 0.3f, 1.0f), // Dark blue
          spitfire::math::cColour3(0.3f, 0.4f, 0.6f), // Light blue
          spitfire::math::cColour3(1.0f, 1.0f, 0.0f), // Yellow
          spitfire::math::cColour3(1.0f, 0.0f, 0.0f), // Red
          spitfire::math::cColour3(0.1f, 0.1f, 0.1f), // Dark grey
        };
        
        pContext->SetShaderConstant("u_numberSamples", (unsigned int)(1 << g_m));
        pContext->SetShaderConstant("u_m", g_m);

        // Results are in range [0.0 1.0] and not [0.0, 1.0[.
        pContext->SetShaderConstant("u_binaryFractionFactor", 1.0f / (powf(2.0f, (GLfloat)g_m) - 1.0f));


        pContext->BindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);
        // First column
        for (size_t index = 0; index < 5; index++) {
          const spitfire::math::cMat4& matTranslation = matTranslationArray[(7 * columns) + index];
          const spitfire::math::cMat4 matModelView = matView * matTranslation;
          pContext->SetShaderConstant("cameraPos", spitfire::math::cVec3());// matModelView * camera.GetPosition());
          //pContext->SetShaderConstant("cameraPos", camera.GetPosition());

          pContext->SetShaderConstant("u_colorMaterial", colours[index]);

          // Roughness of material
          pContext->SetShaderConstant("u_roughnessMaterial", roughness[index]);
          pContext->SetShaderConstant("u_R0Material", R0[index]);

          pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslation);
          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectLargeTeapot);
        }

        pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectLargeTeapot);

        pContext->UnBindTexture(0, textureDiffuse);

        pContext->UnBindShader(shaderBRDF);
      }

      opengl::cSystem::GetErrorString();

      // Render an extra textured teapot under the smoke
      {
        pContext->BindShader(shaderCrate);

        pContext->BindTexture(0, textureDiffuse);
        pContext->BindTexture(1, textureLightMap);
        pContext->BindTexture(2, textureDetail);

        {
          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationSmoke);
            pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectTeapot3);
          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectTeapot3);
        }

        pContext->UnBindTexture(2, textureDetail);
        pContext->UnBindTexture(1, textureLightMap);
        pContext->UnBindTexture(0, textureDiffuse);

        pContext->UnBindShader(shaderCrate);
      }

      // Render the test images
      {
        const size_t n = testImages.size();
        for (size_t i = 0; i < n; i++) {
          pContext->BindShader(shaderPassThrough);

          cTextureVBOPair* pPair = testImages[i];

          pContext->BindTexture(0, pPair->texture);

          pContext->BindStaticVertexBufferObject(pPair->vbo);

          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matView * matTranslationTestImages[i]);

          pContext->DrawStaticVertexBufferObjectTriangles(pPair->vbo);

          pContext->UnBindStaticVertexBufferObject(pPair->vbo);

          pContext->UnBindTexture(0, pPair->texture);

          pContext->UnBindShader(shaderPassThrough);
        }
      }


      // Render some gui in 3d space
      /*{
        pContext->DisableDepthTesting();

        const size_t n = testImages.size();
        for (size_t i = 0; i < n; i++) {
          pContext->BindShader(shaderPassThrough);

          cTextureVBOPair* pPair = testImages[i];

          pContext->BindTexture(0, *(pPair->pTexture));

          pContext->BindStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);

          const spitfire::math::cVec3 position(2.5f - float(i), 1.8f, -5.0f);
          spitfire::math::cMat4 matTranslation;
          matTranslation.SetTranslation(position);

          pContext->SetShaderProjectionAndModelViewMatrices(matProjection, matTranslation);

          pContext->DrawStaticVertexBufferObjectTriangles(staticVertexBufferObjectGuiRectangle);

          pContext->UnBindStaticVertexBufferObject(staticVertexBufferObjectGuiRectangle);

          pContext->UnBindTexture(0, *(pPair->pTexture));

          pContext->UnBindShader(shaderPassThrough);
        }

        pContext->EnableDepthTesting();
      }*/

      pContext->EndRenderToTexture(fbo);

      std::swap(outputFBO, inputFBO);
    }


    // Copy the scene frame buffer object to our depth frame buffer object (We actually render it to get the copy, apparently rendering is faster than glBlitFrameBuffer)
    // NOTE: The target frame buffer object actually has a colour and depth buffer attached, but we only care about the depth buffer
    {
      const spitfire::math::cColour clearColour(0.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(textureFrameBufferObjectScreenDepth);

      pContext->BindShader(shaderScreenRectColourAndDepth);

      pContext->BindTexture(0, textureFrameBufferObjectScreenColourAndDepth[inputFBO]);
      pContext->BindTextureDepthBuffer(1, textureFrameBufferObjectScreenColourAndDepth[inputFBO]);

      RenderScreenRectangleShaderAndTextureAlreadySet();

      pContext->UnBindTextureDepthBuffer(1, textureFrameBufferObjectScreenColourAndDepth[inputFBO]);
      pContext->UnBindTexture(0, textureFrameBufferObjectScreenColourAndDepth[inputFBO]);

      pContext->UnBindShader(shaderScreenRectColourAndDepth);

      pContext->EndRenderToTexture(textureFrameBufferObjectScreenDepth);
    }


    // Ping pong to the other texture so that we can render our particle systems now that we have a depth buffer
    {
      opengl::cTextureFrameBufferObject& fbo = textureFrameBufferObjectScreenColourAndDepth[outputFBO];
      opengl::cTextureFrameBufferObject& fboLastRendered = textureFrameBufferObjectScreenColourAndDepth[inputFBO];

      // Render the scene into a texture for later
      pContext->SetClearColour(skyColour);

      pContext->BeginRenderToTexture(fbo);

      // Now draw an overlay of our rendered textures
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);
        // Draw the existing scene
        glDepthMask(GL_FALSE);
        RenderScreenRectangle(0.5f, 0.5f, staticVertexBufferObjectScreenRectScreen, fboLastRendered, shaderScreenRect);
        glDepthMask(GL_TRUE);
      pContext->EndRenderMode2D();


      if (bIsWireframe) pContext->EnableWireframe();

      // Render the lights
      {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        struct TextureAndSizePair {
          opengl::cTexture* pTexture;
          float fSize;
        };
        const TextureAndSizePair pairs[] = {
          //{ &light.texture, 1.0f },
          { &textureFlare, 16.0f },
        };

        pContext->BindShader(light.shader);

        pContext->BindStaticVertexBufferObject(light.vbo);

        const size_t n = countof(pairs);

        // Green directional light
        for (size_t i = 0; i < n; i++) {
          pContext->BindTexture(0, *pairs[i].pTexture);

          pContext->SetShaderConstant("fSize", pairs[i].fSize);

          pContext->SetShaderConstant("lightColour", util::ChangeLuminance(lightDirectionalDiffuseColour, fLightLuminanceIncrease0To1));

          spitfire::math::cMat4 matTranslation;
          matTranslation.SetTranslation(lightDirectionalPosition);
          pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslation);

          pContext->DrawStaticVertexBufferObjectTriangles(light.vbo);

          pContext->UnBindTexture(0, *pairs[i].pTexture);
        }

        // Red point light
        if (!bIsCameraAtLightSource) {
          for (size_t i = 0; i < n; i++) {
            pContext->BindTexture(0, *pairs[i].pTexture);

            pContext->SetShaderConstant("fSize", pairs[i].fSize);

            pContext->SetShaderConstant("lightColour", util::ChangeLuminance(lightPointColour, fLightLuminanceIncrease0To1));

            spitfire::math::cMat4 matTranslation;
            matTranslation.SetTranslation(lightPointPosition);
            pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslation);

            pContext->DrawStaticVertexBufferObjectTriangles(light.vbo);

            pContext->UnBindTexture(0, *pairs[i].pTexture);
          }
        }

        // Blue spot light
        for (size_t i = 0; i < n; i++) {
          pContext->BindTexture(0, *pairs[i].pTexture);

          pContext->SetShaderConstant("fSize", pairs[i].fSize);

          pContext->SetShaderConstant("lightColour", util::ChangeLuminance(lightSpotColour, fLightLuminanceIncrease0To1));

          spitfire::math::cMat4 matTranslation;
          matTranslation.SetTranslation(lightSpotPosition);
          pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslation);

          pContext->DrawStaticVertexBufferObjectTriangles(light.vbo);

          pContext->UnBindTexture(0, *pairs[i].pTexture);
        }

        pContext->UnBindStaticVertexBufferObject(light.vbo);

        pContext->UnBindShader(light.shader);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
      }

      // Render smoke
      {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        pContext->BindShader(shaderSmoke);

        pContext->BindTexture(0, smoke.texture);
        pContext->BindTextureDepthBuffer(1, fboLastRendered);

        // We split up the object rotation from the normal model matrix so that we can apply it to each particle in the VBO, otherwise we would accidentally rotate each particle into an unknown rotation and billboarding wouldn't work
        pContext->SetShaderConstant("matObjectRotation", matObjectRotation);

        const spitfire::math::cColour3 blue(0.4f, 0.78f, 1.0f);
        const spitfire::math::cColour3 white(1.0f, 1.0f, 1.0f);
        pContext->SetShaderConstant("ambientColour", blue);
        pContext->SetShaderConstant("colour", white);

        const spitfire::math::cColour3 pink(0.83f, 0.19f, 0.55f);
        pContext->SetShaderConstant("lightColour", pink);
        pContext->SetShaderConstant("lightPosition", lightDirectionalPosition);

        pContext->BindStaticVertexBufferObject(smoke.vbo);

        {
          pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationSmoke * matObjectRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(smoke.vbo);
        }

        pContext->UnBindStaticVertexBufferObject(smoke.vbo);

        pContext->UnBindTextureDepthBuffer(1, fboLastRendered);
        pContext->UnBindTexture(0, smoke.texture);

        pContext->UnBindShader(shaderSmoke);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
      }

      // Render fire
      {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        pContext->BindShader(shaderFire);

        pContext->BindTexture(0, fire.texture);
        pContext->BindTextureDepthBuffer(1, fboLastRendered);

        // We split up the object rotation from the normal model matrix so that we can apply it to each particle in the VBO, otherwise we would accidentally rotate each particle into an unknown rotation and billboarding wouldn't work
        pContext->SetShaderConstant("matObjectRotation", matObjectRotation);

        pContext->BindStaticVertexBufferObject(fire.vbo);

        {
          pContext->SetShaderProjectionAndViewAndModelMatrices(matProjection, matView, matTranslationFire * matObjectRotation);

          pContext->DrawStaticVertexBufferObjectTriangles(fire.vbo);
        }

        pContext->UnBindStaticVertexBufferObject(fire.vbo);

        pContext->UnBindTextureDepthBuffer(1, fboLastRendered);
        pContext->UnBindTexture(0, fire.texture);

        pContext->UnBindShader(shaderFire);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
      }

      pContext->EndRenderToTexture(fbo);

      std::swap(outputFBO, inputFBO);
    }

    // Apply depth of field and bokeh
    if (bIsDOFBokeh) {
      dofBokeh.Render(*this, *pContext, textureFrameBufferObjectScreenColourAndDepth[inputFBO], textureFrameBufferObjectScreenDepth, textureFrameBufferObjectScreenColourAndDepth[outputFBO]);
      std::swap(outputFBO, inputFBO);
    }

    // Process HDR bloom
    if (bIsHDR) {
      hdr.RenderBloom(*this, currentTime, *pContext, textureFrameBufferObjectScreenColourAndDepth[inputFBO], textureFrameBufferObjectScreenColourAndDepth[outputFBO]);
      std::swap(outputFBO, inputFBO);
    }

    // Apply lens flare with dirt specks
    if (bIsLensFlareDirt) {
      const float fExposure = hdr.GetExposure();
      lensFlareDirt.Render(*this, *pContext, textureFrameBufferObjectScreenColourAndDepth[inputFBO], textureFrameBufferObjectScreenColourAndDepth[outputFBO], fExposure, bDebugShowFlareOnly);
      std::swap(outputFBO, inputFBO);
    }

    // Process HDR tone mapping
    if (bIsHDR) {
      hdr.RenderToneMapping(*this, currentTime, *pContext, textureFrameBufferObjectScreenColourAndDepth[inputFBO], textureFrameBufferObjectScreenColourAndDepth[outputFBO]);
      std::swap(outputFBO, inputFBO);
    }

    {
      // Render the frame buffer objects to the screen
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToScreen();

      // Now draw an overlay of our rendered textures
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);


      opengl::cTextureFrameBufferObject& fbo = textureFrameBufferObjectScreenColourAndDepth[inputFBO];

      if (GetActiveSimplePostRenderShadersCount() != 0) {
        if (bIsSplitScreenSimplePostEffectShaders) {
          // Draw the screen texture
          RenderScreenRectangle(0.5f, 0.5f, staticVertexBufferObjectScreenRectScreen, fbo, shaderScreenRect);

          // Draw the shaders split screen texture
          RenderScreenRectangle(0.25f, 0.5f, staticVertexBufferObjectScreenRectHalfScreen, fbo, shaderScreenRectSimplePostRender);
        } else {
          // Draw the shaders screen texture
          RenderScreenRectangle(0.5f, 0.5f, staticVertexBufferObjectScreenRectScreen, fbo, shaderScreenRectSimplePostRender);
        }
      } else {
        // Draw the screen texture
        RenderScreenRectangle(0.5f, 0.5f, staticVertexBufferObjectScreenRectScreen, fbo, shaderScreenRect);
      }


      spitfire::math::cVec2 position(0.0f + (0.5f * 0.25f), 0.0f + (0.5f * 0.25f));

      #if 0
      // Draw the lens flare and dirt textures for debugging purposes
      // Down the side
      RenderScreenRectangle(position.x, position.y, staticVertexBufferObjectScreen2DTeapot, lensFlareDirt.GetTextureLensColor(), shaderScreen1D); position.y += 0.25f;
      RenderDebugScreenRectangleVariableSize(position.x, position.y, lensFlareDirt.GetTextureLensDirt()); position.y += 0.25f;
      RenderScreenRectangle(position.x, position.y, staticVertexBufferObjectScreen2DTeapot, lensFlareDirt.GetTextureLensStar(), shaderScreen2D); position.y += 0.25f;

      // Along the bottom
      position.Set(0.0f + (0.5f * 0.25f), 0.75f + (0.5f * 0.25f));
      RenderDebugScreenRectangleVariableSize(position.x, position.y, lensFlareDirt.GetTempA()); position.x += 0.25f;
      RenderDebugScreenRectangleVariableSize(position.x, position.y, lensFlareDirt.GetTempB()); position.x += 0.25f;
      RenderDebugScreenRectangleVariableSize(position.x, position.y, lensFlareDirt.GetTempC()); position.x += 0.25f;
      #endif

      #if 0
      // Draw the scene colour and depth buffer textures for debugging purposes
      float x = 0.125f;
      float y = 0.125f;
      RenderScreenRectangle(x, y, staticVertexBufferObjectScreenRectTeapot, *pTextureFrameBufferObjectScreenDepth, shaderScreenRect); x += 0.25f;
      RenderScreenRectangleDepthTexture(x, y, staticVertexBufferObjectScreenRectTeapot, *pTextureFrameBufferObjectScreenDepth, shaderScreenRect); x += 0.25f;

      // Render the HDR textures for debugging purposes
      RenderScreenRectangle(x, y, staticVertexBufferObjectScreenRectTeapot, textureFrameBufferObjectScreenColourAndDepth[outputFBO], shaderScreenRect); x += 0.25f;
      RenderScreenRectangle(x, y, staticVertexBufferObjectScreenRectTeapot, textureFrameBufferObjectScreenColourAndDepth[inputFBO], shaderScreenRect); x += 0.25f;
      x = 0.125f;
      y += 0.25f;
      RenderScreenRectangle(x, y, staticVertexBufferObjectScreenRectTeapot, *hdr.LuminanceBuffer, shaderScreenRect); x += 0.25f;
      for (size_t i = 0; i < 5; i++) {
        RenderScreenRectangle(x, y, staticVertexBufferObjectScreenRectTeapot, *(hdr.MinificationBuffer[i].pTexture), shaderScreenRect); x += 0.25f;
      }
      x = 0.125f;
      y += 0.25f;
      RenderScreenRectangle(x, y, staticVertexBufferObjectScreenRectTeapot, *hdr.BrightPixelsBuffer, shaderScreenRect); x += 0.25f;
      x = 0.125f;
      y += 0.25f;
      for (size_t i = 0; i < 12; i++) {
        RenderScreenRectangle(x, y, staticVertexBufferObjectScreenRectTeapot, *(hdr.BloomBuffer[i].pTexture), shaderScreenRect); x += 0.25f;
      }
      #endif

      #if 1
      // Draw the shadow map depth texture
      RenderScreenRectangleDepthTexture(position.x, position.y, staticVertexBufferObjectScreenRectTeapot, shadowMapping.GetShadowMapTexture(), shaderScreenRectDepthShadow);
      position += spitfire::math::cVec2(0.25f, 0.0f);

      // Draw the scene depth texture
      RenderScreenRectangleDepthTexture(position.x, position.y, staticVertexBufferObjectScreenRectTeapot, textureFrameBufferObjectScreenColourAndDepth[outputFBO], shaderScreenRect);
      position += spitfire::math::cVec2(0.25f, 0.0f);

      // Draw the particles depth texture
      RenderScreenRectangleDepthTexture(position.x, position.y, staticVertexBufferObjectScreenRectTeapot, textureFrameBufferObjectScreenColourAndDepth[inputFBO], shaderScreenRect);
      position += spitfire::math::cVec2(0.25f, 0.0f);

      // Draw the teapot texture
      RenderScreenRectangle(position.x, position.y, staticVertexBufferObjectScreenRectTeapot, textureFrameBufferObjectTeapot, shaderScreenRect);
      position += spitfire::math::cVec2(0.25f, 0.0f);
      #endif


      // Draw the text overlay
      {
        pContext->BindFont(font);

        // Rendering the font in the middle of the screen
        spitfire::math::cMat4 matModelView;
        matModelView.SetTranslation(0.02f, 0.05f, 0.0f);

        pContext->SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN, matModelView);

        pContext->BindStaticVertexBufferObject2D(textVBO);

        {
          pContext->DrawStaticVertexBufferObjectTriangles2D(textVBO);
        }

        pContext->UnBindStaticVertexBufferObject2D(textVBO);

        pContext->UnBindFont(font);
      }

      pContext->EndRenderMode2D();

      pContext->EndRenderToScreen(*pWindow);
    }

    // Gather our frames per second
    Frames++;
    {
      spitfire::durationms_t t = spitfire::util::GetTimeMS();
      if (t - T0 >= 1000) {
        float seconds = (t - T0) / 1000.0f;
        fFPS = Frames / seconds;
        //LOG(Frames, " frames in ", seconds, " seconds = ", fFPS, " FPS");
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


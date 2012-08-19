#include <cassert>
#include <cmath>
#include <cstring>

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


#include <spitfire/math/geometry.h>

#ifdef __WIN__
typedef HWND windowhandle_t;
typedef HWND controlhandle_t;
#elif defined(__LINUX__)
//typedef GtkWidget* windowhandle_t;
//typedef GtkWidget* controlhandle_t;
#elif defined(__APPLE__)
typedef WindowRef windowhandle_t;
typedef HIViewRef controlhandle_t;
#endif

class cScreen
{
public:
  size_t GetMonitorCount() const;

  size_t GetMonitorContainingCursor() const;
  //size_t GetMonitorContainingWindow(windowhandle_t handle) const;

  bool GetMonitorProperties(size_t monitor, spitfire::math::cRectangle& rect, size_t& colorDepthBits) const;

  bool GetScreenShotOfMonitor(size_t monitor, voodoo::cImage& image) const;
};

size_t cScreen::GetMonitorCount() const
{
  Display* pDisplay = XOpenDisplay(NULL);
  if (pDisplay == nullptr) {
    std::cout<<"cScreen::GetMonitorCount XOpenDisplay FAILED, returning 0"<<std::endl;
    return 0;
  }

  int iScreenCount = XScreenCount(pDisplay);

  XCloseDisplay(pDisplay);
  pDisplay = nullptr;

  //std::cout<<"cScreen::GetMonitorCount returning "<<iScreenCount<<std::endl;
  assert(iScreenCount >= 0);
  return size_t(iScreenCount);
}

size_t cScreen::GetMonitorContainingCursor() const
{
  return 0;
}

bool cScreen::GetMonitorProperties(size_t monitor, spitfire::math::cRectangle& rect, size_t& colorDepthBits) const
{
  Display* pDisplay = XOpenDisplay(NULL);
  if (pDisplay == nullptr) {
    std::cout<<"cScreen::GetMonitorProperties XOpenDisplay FAILED, returning false"<<std::endl;
    return false;
  }

  int screen = monitor;

  //Window XRootWindow(pDisplay, monitor);

  //rect.x = DisplayX(pDisplay, screen);
  //rect.y = DisplayY(pDisplay, screen);
  rect.x = 0;
  rect.y = 0;
  rect.width = DisplayWidth(pDisplay, screen);
  rect.height = DisplayHeight(pDisplay, screen);

  colorDepthBits = DefaultDepth(pDisplay, screen);

  XCloseDisplay(pDisplay);
  pDisplay = nullptr;

  return true;
}

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

bool cScreen::GetScreenShotOfMonitor(size_t monitor, voodoo::cImage& image) const
{
  std::cout<<"cScreen::GetScreenShotOfMonitor"<<std::endl;

#if 0
  const size_t n = GetMonitorCount();
  for (monitor = 0; monitor < n; monitor++) {
    int x = 0;
    int y = 0;
    unsigned int width = 0;
    unsigned int height = 0;

    Display* pDisplay = XOpenDisplay(NULL);
    assert(pDisplay != nullptr);
    Window root = RootWindow(pDisplay, 0);
    int num_sizes = 0;
    std::cout<<"cScreen::GetScreenShotOfMonitor a"<<std::endl;
    XRRScreenSize* xrrs = XRRSizes(pDisplay, 0, &num_sizes);
    std::cout<<"cScreen::GetScreenShotOfMonitor b"<<std::endl;
    XRRScreenConfiguration* pConfiguration = XRRGetScreenInfo(pDisplay, root);
    std::cout<<"cScreen::GetScreenShotOfMonitor c"<<std::endl;
    //short original_rate = XRRConfigCurrentRate(pConfiguration);
    Rotation original_rotation;
    SizeID original_size_id = XRRConfigCurrentConfiguration(pConfiguration, &original_rotation);
    std::cout<<"cScreen::GetScreenShotOfMonitor d"<<std::endl;

    width = xrrs[original_size_id].width;
    height = xrrs[original_size_id].height;

    /*unsigned int border_width = 0;
    unsigned int depth = 0;
    Window root = RootWindow(pDisplay, monitor);
    if (XGetGeometry(pDisplay, root, &root, &x, &y, &width, &height, &border_width, &depth) == false) {
      std::cout<<"cScreen::GetScreenShotOfMonitor XGetGeometry FAILED, returning false"<<std::endl;
      assert(false);
      return false;
    }*/

    std::cout<<"cScreen::GetScreenShotOfMonitor "<<x<<","<<y<<" "<<width<<"x"<<height<<std::endl;

    /*GdkScreen *screen;
    int num_monitors;
    int i;

    screen = gdk_screen_get_default ();
    num_monitors = gdk_screen_get_n_monitors ();

    for (i = 0; i < num_monitors; i++) {
      GdkRectangle rect;

      gdk_screen_get_monitor_geometry (screen, i, &rect);
      printf ("monitor %d: offsets (%d, %d), size (%d, %d)\n",
      i,
      rect.x, rect.y,
      rect.width, rect.height);
    }*/

    assert(false);
  }
#endif

  Display* pDisplay = XOpenDisplay(NULL);
  if (pDisplay == nullptr) {
    std::cout<<"cScreen::GetScreenShotOfMonitor XOpenDisplay FAILED, returning false"<<std::endl;
    return false;
  }

  //int screen = DefaultScreen(pDisplay);
  int screen = monitor;

  //const size_t x = DisplayX(pDisplay, screen);
  //const size_t y = DisplayY(pDisplay, screen);
  const size_t x = 0;
  const size_t y = 0;
  const size_t width = DisplayWidth(pDisplay, screen);
  const size_t height = DisplayHeight(pDisplay, screen);

  //const size_t colorDepthBits = DefaultDepth(pDisplay, screen);

  XImage* pImage = XGetImage(
    pDisplay, RootWindow(pDisplay, screen),
    x, y, width, height,
    AllPlanes, ZPixmap
  );
  if (pImage == nullptr) {
    std::cout<<"cScreen::GetScreenShotOfMonitor XGetImage FAILED, returning false"<<std::endl;
    return false;
  }

  std::vector<uint8_t> buffer;
  buffer.resize(width * height * 4, 0);

  for (size_t y = 0; (y < height); y++) {
    for (size_t x = 0; (x < width); x++) {
      int index = ((y * width) + x) * 4;

      bool bIsFlip = true;/*!(
        (y > height / 4 - 1) && (y < 3 * height / 4) &&
        (x > width / 4 - 1)  && (x < 3 * width / 4)
      );*/

      size_t pixel = XGetPixel(pImage, x, y);
      uint8_t red = (pixel & pImage->red_mask) >> 16;
      uint8_t green = (pixel & pImage->green_mask) >> 8;
      uint8_t blue = (pixel & pImage->blue_mask) >> 0;

      buffer[index] = bIsFlip ? red : 255 - red;
      buffer[index + 1] = bIsFlip ? green : 255 - green;
      buffer[index + 2] = bIsFlip ? blue : 255 - blue;
      buffer[index + 3] = 255;
    }
  }

  XDestroyImage(pImage);
  pImage = nullptr;

  XCloseDisplay(pDisplay);
  pDisplay = nullptr;

  assert(width == image.GetWidth());
  assert(height == image.GetHeight());
  assert(opengl::PIXELFORMAT::R8G8B8A8 == image.GetPixelFormat());
  image.CreateFromBuffer(buffer.data(), width, height, opengl::PIXELFORMAT::R8G8B8A8);

  image.FlipVertically();

  std::cout<<"cScreen::GetScreenShotOfMonitor returning true"<<std::endl;
  return true;
}


class cHeightmapData
{
public:
  explicit cHeightmapData(const opengl::string_t& sFilename);

  size_t GetWidth() const { return width; }
  size_t GetDepth() const { return depth; }

  float GetHeight(size_t x, size_t y) const;
  spitfire::math::cVec3 GetNormal(size_t x, size_t y, const spitfire::math::cVec3& scale) const;

private:
  spitfire::math::cVec3 GetNormalOfTriangle(const spitfire::math::cVec3& p0, const spitfire::math::cVec3& p1, const spitfire::math::cVec3& p2) const;

  std::vector<float> heightmap;
  size_t width;
  size_t depth;
};

cHeightmapData::cHeightmapData(const opengl::string_t& sFilename)
{
  voodoo::cImage image;
  if (!image.LoadFromFile(sFilename)) {
    std::cout<<"cHeightmapData::cHeightmapData Could not load "<<opengl::string::ToUTF8(sFilename)<<std::endl;
    assert(false);
  }

  width = image.GetWidth();
  depth = image.GetHeight();

  // Create heightmap data
  const size_t n = width * depth;

  heightmap.resize(n, 0);

  const uint8_t* pPixels = image.GetPointerToBuffer();

  const float fOneOver255 = 1.0f / 255.0f;

  for (size_t y = 0; y < depth; y++) {
    for (size_t x = 0; x < width; x++) {
      heightmap[(y * width) + x] = float(pPixels[(y * width) + x]) * fOneOver255;
    }
  }
}

float cHeightmapData::GetHeight(size_t x, size_t y) const
{
  assert(((y * width) + x) < heightmap.size());
  return heightmap[(y * depth) + x];
}

spitfire::math::cVec3 cHeightmapData::GetNormalOfTriangle(const spitfire::math::cVec3& p0, const spitfire::math::cVec3& p1, const spitfire::math::cVec3& p2) const
{
  const spitfire::math::cVec3 v0 = p1 - p0;
  const spitfire::math::cVec3 v1 = p2 - p0;

  return v0.CrossProduct(v1);
}

spitfire::math::cVec3 cHeightmapData::GetNormal(size_t x, size_t y, const spitfire::math::cVec3& scale) const
{
  assert(((y * depth) + x) < heightmap.size());

  // Get the height of the target point and the 4 heights in a cross shape around the target
  spitfire::math::cVec3 points[5];

  points[0] = points[1] = points[2] = points[3] = points[4] = spitfire::math::cVec3(float(x), float(y), GetHeight(x, y));

  // First column
  if (x != 0) points[0] = spitfire::math::cVec3(float(x - 1), float(y), GetHeight(x - 1, y));

  // Second column
  if ((x != 0) && (y != 0)) points[1] = spitfire::math::cVec3(float(x), float(y - 1), GetHeight(x, y - 1));
  //points[2] = spitfire::math::cVec3(float(x), float(y), GetHeight(x, y));
  if ((y + 1) < depth) points[3] = spitfire::math::cVec3(float(x), float(y + 1), GetHeight(x, y + 1));

  // Third column
  if ((x + 1) < width) points[4] = spitfire::math::cVec3(float(x + 1), float(y), GetHeight(x + 1, y));

  points[0] *= scale;
  points[1] *= scale;
  points[2] *= scale;
  points[3] *= scale;
  points[4] *= scale;

  spitfire::math::cVec3 normal;

  normal += GetNormalOfTriangle(points[0], points[2], points[1]);
  normal += GetNormalOfTriangle(points[0], points[2], points[3]);
  normal += GetNormalOfTriangle(points[4], points[2], points[1]);
  normal += GetNormalOfTriangle(points[4], points[2], points[3]);

  normal.Normalise();

  return normal;
}


class cTextureVertexBufferObjectPair
{
public:
  cTextureVertexBufferObjectPair();
  ~cTextureVertexBufferObjectPair();

  bool IsValid() const;

  opengl::cTexture* pTexture;
  opengl::cStaticVertexBufferObject* pVBO;
};

cTextureVertexBufferObjectPair::cTextureVertexBufferObjectPair() :
  pTexture(nullptr),
  pVBO(nullptr)
{
}

cTextureVertexBufferObjectPair::~cTextureVertexBufferObjectPair()
{
  assert(pTexture == nullptr);
  assert(pVBO == nullptr);
}

bool cTextureVertexBufferObjectPair::IsValid() const
{
  return (pTexture != nullptr) && (pTexture->IsValid()) && (pVBO != nullptr) && (pVBO->IsCompiled());
}


class cApplication : public opengl::cWindowEventListener, public opengl::cInputEventListener
{
public:
  cApplication();
  ~cApplication();

  bool Create();
  void Destroy();

  void Run();

private:
  void CreateHeightmapQuads(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const cHeightmapData& data, const spitfire::math::cVec3& scale);
  void CreateHeightmapQuadsIndexed(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const cHeightmapData& data, const spitfire::math::cVec3& scale);

  void CreateVegetation(const cHeightmapData& data, const spitfire::math::cVec3& scale);

  void CreateScreenBlendQuadVBO(float fRatioOfTextureWidthToScreenShotWidth, float fRatioOfTextureHeightToScreenShotHeight);
  void CreateScreenQuadVBO();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  bool bIsWireframe;
  bool bUseQuadsIndexed;
  bool bIsMouseMovement;
  bool bIsDone;

  size_t width;
  size_t depth;
  spitfire::math::cVec3 scale;

  // For controlling the rotation and zoom of the trackball
  spitfire::math::cQuaternion rotationX;
  spitfire::math::cQuaternion rotationZ;
  float fZoom;

  opengl::cSystem system;

  opengl::cWindow* pWindow;

  opengl::cContext* pContext;

  opengl::cTextureFrameBufferObject* pTextureFrameBufferObject;

  opengl::cTexture* pTextureScreenShot;
  opengl::cTexture* pTextureDiffuse;
  opengl::cTexture* pTextureLightMap;
  opengl::cTexture* pTextureDetail;

  opengl::cShader* pShaderHeightmap;
  opengl::cShader* pShaderMask;
  opengl::cShader* pShaderPassThroughWithColour;
  opengl::cShader* pShaderScreenBlendQuad;
  opengl::cShader* pShaderScreenQuad;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectHeightmapQuads;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectHeightmapQuadsIndexed;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectScreenBlendQuad;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectScreenQuad;

  cTextureVertexBufferObjectPair grass;
  cTextureVertexBufferObjectPair flowers;
  cTextureVertexBufferObjectPair trees;
  cTextureVertexBufferObjectPair rocks;
};

cApplication::cApplication() :
  bIsWireframe(false),
  bUseQuadsIndexed(true),
  bIsMouseMovement(false),
  bIsDone(false),

  width(0),
  depth(0),

  fZoom(0.0f),

  pWindow(nullptr),
  pContext(nullptr),

  pTextureFrameBufferObject(nullptr),

  pTextureScreenShot(nullptr),
  pTextureDiffuse(nullptr),
  pTextureLightMap(nullptr),
  pTextureDetail(nullptr),

  pShaderHeightmap(nullptr),
  pShaderMask(nullptr),
  pShaderPassThroughWithColour(nullptr),
  pShaderScreenBlendQuad(nullptr),
  pShaderScreenQuad(nullptr),

  pStaticVertexBufferObjectHeightmapQuads(nullptr),
  pStaticVertexBufferObjectHeightmapQuadsIndexed(nullptr),

  pStaticVertexBufferObjectScreenBlendQuad(nullptr),
  pStaticVertexBufferObjectScreenQuad(nullptr)
{
}

cApplication::~cApplication()
{
  Destroy();
}

void cApplication::CreateHeightmapQuads(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const cHeightmapData& data, const spitfire::math::cVec3& scale)
{
  assert(pStaticVertexBufferObject != nullptr);

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoordinates;
  //std::vector<uint16_t> indices;

  const float fDetailMapRepeat = 10.0f;
  const float fDetailMapWidth = fDetailMapRepeat;

  // NOTE: Diffuse and lightmap will have the duplicated texture coordinates (0..1)
  // Detail map will have repeated texture coordinates (0..fDetailMapRepeat)
  opengl::cGeometryBuilder_v3_n3_t2_t2_t2 builder(vertices, normals, textureCoordinates);

  const size_t width = data.GetWidth();
  const size_t depth = data.GetDepth();
  for (size_t y = 0; y < depth - 1; y++) {
    for (size_t x = 0; x < width - 1; x++) {
      const float fDiffuseAndLightmapU = float(x) / float(width);
      const float fDiffuseAndLightmapV = float(y) / float(depth);
      const float fDiffuseAndLightmapU2 = (float(x) + 1.0f) / float(width);
      const float fDiffuseAndLightmapV2 = (float(y) + 1.0f) / float(depth);

      const float fDetailMapU = float(x) / float(width) * fDetailMapWidth;
      const float fDetailMapV = float(y) / float(depth) * fDetailMapWidth;
      const float fDetailMapU2 = (float(x) + 1.0f) / float(width) * fDetailMapWidth;
      const float fDetailMapV2 = (float(y) + 1.0f) / float(depth) * fDetailMapWidth;

      builder.PushBack(scale * spitfire::math::cVec3(float(x), float(y), data.GetHeight(x, y)), data.GetNormal(x, y, scale), spitfire::math::cVec2(fDiffuseAndLightmapU, fDiffuseAndLightmapV), spitfire::math::cVec2(fDiffuseAndLightmapU, fDiffuseAndLightmapV), spitfire::math::cVec2(fDetailMapU, fDetailMapV));
      builder.PushBack(scale * spitfire::math::cVec3(float(x + 1), float(y), data.GetHeight(x + 1, y)), data.GetNormal(x + 1, y, scale), spitfire::math::cVec2(fDiffuseAndLightmapU2, fDiffuseAndLightmapV), spitfire::math::cVec2(fDiffuseAndLightmapU2, fDiffuseAndLightmapV), spitfire::math::cVec2(fDetailMapU2, fDetailMapV));
      builder.PushBack(scale * spitfire::math::cVec3(float(x + 1), float(y + 1), data.GetHeight(x + 1, y + 1)), data.GetNormal(x + 1, y + 1, scale), spitfire::math::cVec2(fDiffuseAndLightmapU2, fDiffuseAndLightmapV2), spitfire::math::cVec2(fDiffuseAndLightmapU2, fDiffuseAndLightmapV2), spitfire::math::cVec2(fDetailMapU2, fDetailMapV2));
      builder.PushBack(scale * spitfire::math::cVec3(float(x), float(y + 1), data.GetHeight(x, y + 1)), data.GetNormal(x, y + 1, scale), spitfire::math::cVec2(fDiffuseAndLightmapU, fDiffuseAndLightmapV2), spitfire::math::cVec2(fDiffuseAndLightmapU, fDiffuseAndLightmapV2), spitfire::math::cVec2(fDetailMapU, fDetailMapV2));
    }
  }

  pStaticVertexBufferObject->SetVertices(vertices);
  pStaticVertexBufferObject->SetNormals(normals);
  pStaticVertexBufferObject->SetTextureCoordinates(textureCoordinates);
  //pStaticVertexBufferObject->SetIndices(indices);

  pStaticVertexBufferObject->Compile(system);
}

void cApplication::CreateHeightmapQuadsIndexed(opengl::cStaticVertexBufferObject* pStaticVertexBufferObject, const cHeightmapData& data, const spitfire::math::cVec3& scale)
{
  assert(pStaticVertexBufferObject != nullptr);

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textureCoordinates;
  std::vector<uint16_t> indices;

  const float fDetailMapRepeat = 10.0f;
  const float fDetailMapWidth = fDetailMapRepeat;

  // NOTE: Diffuse and lightmap will have the duplicated texture coordinates (0..1)
  // Detail map will have repeated texture coordinates (0..fDetailMapRepeat)
  opengl::cGeometryBuilder_v3_n3_t2_t2_t2_i builder(vertices, normals, textureCoordinates, indices);

  const size_t width = data.GetWidth();
  const size_t depth = data.GetDepth();
  for (size_t y = 0; y < depth; y++) {
    for (size_t x = 0; x < width; x++) {

      const float fDiffuseAndLightmapU = float(x) / float(width);
      const float fDiffuseAndLightmapV = float(y) / float(depth);

      const float fDetailMapU = float(x) / float(width) * fDetailMapWidth;
      const float fDetailMapV = float(y) / float(depth) * fDetailMapWidth;

      builder.PushBack(scale * spitfire::math::cVec3(float(x), float(y), data.GetHeight(x, y)), data.GetNormal(x, y, scale), spitfire::math::cVec2(fDiffuseAndLightmapU, fDiffuseAndLightmapV), spitfire::math::cVec2(fDiffuseAndLightmapU, fDiffuseAndLightmapV), spitfire::math::cVec2(fDetailMapU, fDetailMapV));
    }
  }


  for (size_t y = 0; y < depth - 1; y++) {
    for (size_t x = 0; x < width - 1; x++) {
      builder.PushBackIndex((y * width) + x);
      builder.PushBackIndex((y * width) + (x + 1));
      builder.PushBackIndex(((y + 1) * width) + (x + 1));
      builder.PushBackIndex(((y + 1) * width) + x);
    }
  }

  pStaticVertexBufferObject->SetVertices(vertices);
  pStaticVertexBufferObject->SetNormals(normals);
  pStaticVertexBufferObject->SetTextureCoordinates(textureCoordinates);
  pStaticVertexBufferObject->SetIndices(indices);

  pStaticVertexBufferObject->Compile(system);
}

//void cApplication::CreateHeightmapTriangleStrips();

void cApplication::CreateVegetation(const cHeightmapData& data, const spitfire::math::cVec3& scale)
{
  std::cout<<"cApplication::CreateVegetation"<<std::endl;

  assert(grass.pVBO != nullptr);
  assert(flowers.pVBO != nullptr);
  assert(trees.pVBO != nullptr);
  assert(rocks.pVBO != nullptr);

  voodoo::cImage lightmap;
  if (!lightmap.LoadFromFile(TEXT("textures/lightmap.png"))) {
    std::cout<<"cApplication::CreateVegetation Could not load \"textures/lightmap.png\""<<std::endl;
    assert(false);
  }

  const uint8_t* pLightmapData = lightmap.GetPointerToBuffer();

  const spitfire::math::cVec2 lightmapScale(float(lightmap.GetWidth()) / float(data.GetWidth()), float(lightmap.GetHeight()) / float(data.GetDepth()));

  const float fLightmapBytesPerPixel = lightmap.GetBytesPerPixel();
  const float fLightmapBytesPerRow = lightmap.GetWidth() * lightmap.GetBytesPerPixel();


  voodoo::cImage vegetationmap;
  if (!vegetationmap.LoadFromFile(TEXT("textures/vegetationmap.png"))) {
    std::cout<<"cApplication::CreateVegetation Could not load \"textures/vegetationmap.png\""<<std::endl;
    assert(false);
  }

  const uint8_t* pVegetationmapData = vegetationmap.GetPointerToBuffer();

  const spitfire::math::cVec2 vegetationmapScale(float(vegetationmap.GetWidth()) / float(data.GetWidth()), float(vegetationmap.GetHeight()) / float(data.GetDepth()));

  const float fVegetationmapBytesPerPixel = vegetationmap.GetBytesPerPixel();
  const float fVegetationmapBytesPerRow = vegetationmap.GetWidth() * vegetationmap.GetBytesPerPixel();


  const float fOneOver255 = 1.0f / 255.0f;

  std::vector<float> verticesGrass;
  std::vector<float> normalsGrass;
  std::vector<float> textureCoordinatesGrass;
  //std::vector<uint16_t> indicesGrass;
  std::vector<float> coloursGrass;

  opengl::cGeometryBuilder_v3_n3_t2_c4 builderGrass(verticesGrass, normalsGrass, textureCoordinatesGrass, coloursGrass);

  std::vector<float> verticesTrees;
  std::vector<float> normalsTrees;
  std::vector<float> textureCoordinatesTrees;
  //std::vector<uint16_t> indicesTrees;
  std::vector<float> coloursTrees;

  opengl::cGeometryBuilder_v3_n3_t2_c4 builderTrees(verticesTrees, normalsTrees, textureCoordinatesTrees, coloursTrees);

  std::vector<float> verticesRocks;
  std::vector<float> normalsRocks;
  std::vector<float> textureCoordinatesRocks;
  //std::vector<uint16_t> indicesRocks;
  std::vector<float> coloursRocks;

  opengl::cGeometryBuilder_v3_n3_t2_c4 builderRocks(verticesRocks, normalsRocks, textureCoordinatesRocks, coloursRocks);

  const spitfire::math::cVec3 minGrass(-0.5f, 0.0f, 0.0f);
  const spitfire::math::cVec3 maxGrass(0.5f, 0.0f, 1.0f);
  const spitfire::math::cVec3 minTree(-0.5f, 0.0f, 0.0f);
  const spitfire::math::cVec3 maxTree(0.5f, 0.0f, 2.0f);
  const spitfire::math::cVec3 minRock(-0.5f, 0.0f, 0.0f);
  const spitfire::math::cVec3 maxRock(0.5f, 0.0f, 0.5f);
  const spitfire::math::cVec3 normal(0.0f, 1.0f, 0.0f);

  const spitfire::math::cVec3 axisZ(0.0f, 0.0f, 1.0f);

  const size_t width = data.GetWidth();
  const size_t depth = data.GetDepth();
  for (size_t y = 0; y < depth - 1; y += 10) {
    for (size_t x = 0; x < width - 1; x += 10) {
      const size_t vegetationindex = (vegetationmapScale.y * float(y) * fVegetationmapBytesPerRow) + (vegetationmapScale.x * float(x) * fVegetationmapBytesPerPixel);
      const float fDensityGrass0To1 = pVegetationmapData[vegetationindex] * fOneOver255; // Red channel
      //const float fDensityFlowers0To1 = pVegetationmapData[vegetationindex + 1] * fOneOver255; // Green channel
      const float fDensityTrees0To1 = pVegetationmapData[vegetationindex + 2] * fOneOver255; // Blue channel
      const float fDensityRocks0To1 = pVegetationmapData[vegetationindex + 3] * fOneOver255; // Alpha channel

      uint32_t seedGrass = (y * width) + x;
      spitfire::math::cScopedPredictableRandom generatorGrass(seedGrass);

      //uint32_t seedFlowers = (1 * width * depth) + (y * width) + x;
      //spitfire::math::cScopedPredictableRandom generatorFlowers(seedFlowers);

      uint32_t seedTrees = (2 * width * depth) + (y * width) + x;
      spitfire::math::cScopedPredictableRandom generatorTrees(seedTrees);

      uint32_t seedRocks = (3 * width * depth) + (y * width) + x;
      spitfire::math::cScopedPredictableRandom generatorRocks(seedRocks);

      uint32_t nGrass = fDensityGrass0To1 * 10.0f;
      for (uint32_t iGrass = 0; iGrass < nGrass; iGrass++) {
        float fGrassX = float(x) + 20.0f * (-0.5f + generatorGrass.GetRandomNumber0To1());
        float fGrassY = float(y) + 20.0f * (-0.5f + generatorGrass.GetRandomNumber0To1());
        if ((fGrassX < 0.0f) || (fGrassY < 0.0f) || (fGrassX > float(width)) || (fGrassY > float(depth))) continue;

        const spitfire::math::cVec3 position(scale * spitfire::math::cVec3(fGrassX, fGrassY, data.GetHeight(fGrassX, fGrassY) + 0.01f));
        const float fRotationDegreesOfGrass = generatorGrass.GetRandomNumber0To1() * 360.0f;

        const size_t index = (lightmapScale.y * float(size_t(fGrassY)) * fLightmapBytesPerRow) + (lightmapScale.x * float(size_t(fGrassX)) * fLightmapBytesPerPixel);
        const spitfire::math::cColour colourLightmap(min(pLightmapData[index] * fOneOver255, 0.9f), min(pLightmapData[index + 1] * fOneOver255, 0.9f), min(pLightmapData[index + 2] * fOneOver255, 0.9f));
        const spitfire::math::cColour colour(colourLightmap.r + (0.1f * (-0.5f + generatorGrass.GetRandomNumber0To1())), colourLightmap.g + (0.1f * (-0.5f + generatorGrass.GetRandomNumber0To1())), colourLightmap.b + (0.1f * (-0.5f + generatorGrass.GetRandomNumber0To1())));

        // Create 3 billboards at 60 degree increments
        /* From above they will look something like this:
          \|/
          -*-
          /|\
        */
        for (size_t i = 0; i < 3; i++) {
          const float fRotationDegrees = fRotationDegreesOfGrass + (float(i + 1) * 60.0f);

          // The rotation for this billboard of grass
          spitfire::math::cQuaternion rotationZ;
          rotationZ.SetFromAxisAngleDegrees(axisZ, fRotationDegrees);

          spitfire::math::cMat4 matRotation;
          matRotation.SetRotation(rotationZ);

          const spitfire::math::cVec3 point[4] = {
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minGrass.x, minGrass.y, minGrass.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxGrass.x, minGrass.y, minGrass.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxGrass.x, minGrass.y, maxGrass.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minGrass.x, minGrass.y, maxGrass.z)),
          };

          const spitfire::math::cVec3 normalRotated = matRotation.GetRotatedVec3(normal);

          // Front face
          builderGrass.PushBack(point[0], normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
          builderGrass.PushBack(point[1], normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
          builderGrass.PushBack(point[2], normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
          builderGrass.PushBack(point[3], normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);

          // Back face
          builderGrass.PushBack(point[3], -normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);
          builderGrass.PushBack(point[2], -normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
          builderGrass.PushBack(point[1], -normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
          builderGrass.PushBack(point[0], -normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
        }
      }


      uint32_t nTrees = fDensityTrees0To1 * 10.0f;
      for (uint32_t iTrees = 0; iTrees < nTrees; iTrees++) {
        float fTreesX = float(x) + 20.0f * (-0.5f + generatorTrees.GetRandomNumber0To1());
        float fTreesY = float(y) + 20.0f * (-0.5f + generatorTrees.GetRandomNumber0To1());
        if ((fTreesX < 0.0f) || (fTreesY < 0.0f) || (fTreesX > float(width)) || (fTreesY > float(depth))) continue;

        const spitfire::math::cVec3 position(scale * spitfire::math::cVec3(fTreesX, fTreesY, data.GetHeight(fTreesX, fTreesY) + 0.01f));
        const float fRotationDegreesOfTrees = generatorTrees.GetRandomNumber0To1() * 360.0f;

        const size_t index = (lightmapScale.y * float(size_t(fTreesY)) * fLightmapBytesPerRow) + (lightmapScale.x * float(size_t(fTreesX)) * fLightmapBytesPerPixel);
        const spitfire::math::cColour colourLightmap(min(pLightmapData[index] * fOneOver255, 0.9f), min(pLightmapData[index + 1] * fOneOver255, 0.9f), min(pLightmapData[index + 2] * fOneOver255, 0.9f));
        const spitfire::math::cColour colour(colourLightmap.r + (0.1f * (-0.5f + generatorTrees.GetRandomNumber0To1())), colourLightmap.g + (0.1f * (-0.5f + generatorTrees.GetRandomNumber0To1())), colourLightmap.b + (0.1f * (-0.5f + generatorTrees.GetRandomNumber0To1())));

        // Create 3 billboards at 60 degree increments
        /* From above they will look something like this:
          \|/
          -*-
          /|\
        */
        for (size_t i = 0; i < 3; i++) {
          const float fRotationDegrees = fRotationDegreesOfTrees + (float(i + 1) * 60.0f);

          // The rotation for this billboard of trees
          spitfire::math::cQuaternion rotationZ;
          rotationZ.SetFromAxisAngleDegrees(axisZ, fRotationDegrees);

          spitfire::math::cMat4 matRotation;
          matRotation.SetRotation(rotationZ);

          const spitfire::math::cVec3 point[4] = {
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minTree.x, minTree.y, minTree.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxTree.x, minTree.y, minTree.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxTree.x, minTree.y, maxTree.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minTree.x, minTree.y, maxTree.z)),
          };

          const spitfire::math::cVec3 normalRotated = matRotation.GetRotatedVec3(normal);

          // Front face
          builderTrees.PushBack(point[0], normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
          builderTrees.PushBack(point[1], normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
          builderTrees.PushBack(point[2], normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
          builderTrees.PushBack(point[3], normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);

          // Back face
          builderTrees.PushBack(point[3], -normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);
          builderTrees.PushBack(point[2], -normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
          builderTrees.PushBack(point[1], -normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
          builderTrees.PushBack(point[0], -normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
        }
      }


      uint32_t nRocks = fDensityRocks0To1 * 10.0f;
      for (uint32_t iRocks = 0; iRocks < nRocks; iRocks++) {
        float fRocksX = float(x) + 20.0f * (-0.5f + generatorRocks.GetRandomNumber0To1());
        float fRocksY = float(y) + 20.0f * (-0.5f + generatorRocks.GetRandomNumber0To1());
        if ((fRocksX < 0.0f) || (fRocksY < 0.0f) || (fRocksX > float(width)) || (fRocksY > float(depth))) continue;

        const spitfire::math::cVec3 position(scale * spitfire::math::cVec3(fRocksX, fRocksY, data.GetHeight(fRocksX, fRocksY) + 0.01f));
        const float fRotationDegreesOfRocks = generatorRocks.GetRandomNumber0To1() * 360.0f;

        const size_t index = (lightmapScale.y * float(size_t(fRocksY)) * fLightmapBytesPerRow) + (lightmapScale.x * float(size_t(fRocksX)) * fLightmapBytesPerPixel);
        const spitfire::math::cColour colourLightmap(min(pLightmapData[index] * fOneOver255, 0.9f), min(pLightmapData[index + 1] * fOneOver255, 0.9f), min(pLightmapData[index + 2] * fOneOver255, 0.9f));
        const spitfire::math::cColour colour(colourLightmap.r + (0.1f * (-0.5f + generatorRocks.GetRandomNumber0To1())), colourLightmap.g + (0.1f * (-0.5f + generatorRocks.GetRandomNumber0To1())), colourLightmap.b + (0.1f * (-0.5f + generatorRocks.GetRandomNumber0To1())));

        // Create 3 billboards at 60 degree increments
        /* From above they will look something like this:
          \|/
          -*-
          /|\
        */
        for (size_t i = 0; i < 3; i++) {
          const float fRotationDegrees = fRotationDegreesOfRocks + (float(i + 1) * 60.0f);

          // The rotation for this billboard of rocks
          spitfire::math::cQuaternion rotationZ;
          rotationZ.SetFromAxisAngleDegrees(axisZ, fRotationDegrees);

          spitfire::math::cMat4 matRotation;
          matRotation.SetRotation(rotationZ);

          const spitfire::math::cVec3 point[4] = {
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minRock.x, minRock.y, minRock.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxRock.x, minRock.y, minRock.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(maxRock.x, minRock.y, maxRock.z)),
            position + matRotation.GetRotatedVec3(spitfire::math::cVec3(minRock.x, minRock.y, maxRock.z)),
          };

          const spitfire::math::cVec3 normalRotated = matRotation.GetRotatedVec3(normal);

          // Front face
          builderRocks.PushBack(point[0], normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
          builderRocks.PushBack(point[1], normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
          builderRocks.PushBack(point[2], normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
          builderRocks.PushBack(point[3], normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);

          // Back face
          builderRocks.PushBack(point[3], -normalRotated, spitfire::math::cVec2(0.0f, 0.0f), colour);
          builderRocks.PushBack(point[2], -normalRotated, spitfire::math::cVec2(1.0f, 0.0f), colour);
          builderRocks.PushBack(point[1], -normalRotated, spitfire::math::cVec2(1.0f, 1.0f), colour);
          builderRocks.PushBack(point[0], -normalRotated, spitfire::math::cVec2(0.0f, 1.0f), colour);
        }
      }
    }
  }

  std::cout<<"cApplication::CreateVegetation a"<<std::endl;

  grass.pVBO->SetVertices(verticesGrass);
  grass.pVBO->SetNormals(normalsGrass);
  grass.pVBO->SetTextureCoordinates(textureCoordinatesGrass);
  //grass.pVBO->SetIndices(indicesGrass);
  grass.pVBO->SetColours(coloursGrass);

  grass.pVBO->Compile(system);

  trees.pVBO->SetVertices(verticesTrees);
  trees.pVBO->SetNormals(normalsTrees);
  trees.pVBO->SetTextureCoordinates(textureCoordinatesTrees);
  //trees.pVBO->SetIndices(indicesTrees);
  trees.pVBO->SetColours(coloursTrees);

  trees.pVBO->Compile(system);

  rocks.pVBO->SetVertices(verticesRocks);
  rocks.pVBO->SetNormals(normalsRocks);
  rocks.pVBO->SetTextureCoordinates(textureCoordinatesRocks);
  //rocks.pVBO->SetIndices(indicesRocks);
  rocks.pVBO->SetColours(coloursRocks);

  rocks.pVBO->Compile(system);

  std::cout<<"cApplication::CreateVegetation returning"<<std::endl;
}

void cApplication::CreateScreenBlendQuadVBO(float fRatioOfTextureWidthToScreenShotWidth, float fRatioOfTextureHeightToScreenShotHeight)
{
  assert(pStaticVertexBufferObjectScreenBlendQuad != nullptr);

  std::vector<float> vertices;
  std::vector<float> textureCoordinates;
  //std::vector<uint16_t> indices;

  const float_t fHalfSize = 0.5f;
  const spitfire::math::cVec2 vMin(-fHalfSize, -fHalfSize);
  const spitfire::math::cVec2 vMax(fHalfSize, fHalfSize);

  opengl::cGeometryBuilder_v2_t2_t2 builder(vertices, textureCoordinates);

  // Front facing quad
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMax.y), spitfire::math::cVec2(fRatioOfTextureWidthToScreenShotWidth, 0.0f), spitfire::math::cVec2(1.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(fRatioOfTextureWidthToScreenShotWidth, fRatioOfTextureHeightToScreenShotHeight), spitfire::math::cVec2(1.0f, 1.0f));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMin.y), spitfire::math::cVec2(0.0f, fRatioOfTextureHeightToScreenShotHeight), spitfire::math::cVec2(0.0f, 1.0f));

  pStaticVertexBufferObjectScreenBlendQuad->SetVertices(vertices);
  pStaticVertexBufferObjectScreenBlendQuad->SetTextureCoordinates(textureCoordinates);
  //pStaticVertexBufferObjectScreenBlendQuad->SetIndices(indices);

  pStaticVertexBufferObjectScreenBlendQuad->Compile2D(system);
}

void cApplication::CreateScreenQuadVBO()
{
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);

  std::vector<float> vertices;
  std::vector<float> textureCoordinates;
  //std::vector<uint16_t> indices;

  const float_t fHalfSize = 0.5f;
  const spitfire::math::cVec2 vMin(-fHalfSize, -fHalfSize);
  const spitfire::math::cVec2 vMax(fHalfSize, fHalfSize);

  opengl::cGeometryBuilder_v2_t2 builder(vertices, textureCoordinates);

  // Front facing quad
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMax.y), spitfire::math::cVec2(0.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMax.y), spitfire::math::cVec2(1.0f, 0.0f));
  builder.PushBack(spitfire::math::cVec2(vMax.x, vMin.y), spitfire::math::cVec2(1.0f, 1.0f));
  builder.PushBack(spitfire::math::cVec2(vMin.x, vMin.y), spitfire::math::cVec2(0.0f, 1.0f));

  pStaticVertexBufferObjectScreenQuad->SetVertices(vertices);
  pStaticVertexBufferObjectScreenQuad->SetTextureCoordinates(textureCoordinates);
  //pStaticVertexBufferObjectScreenQuad->SetIndices(indices);

  pStaticVertexBufferObjectScreenQuad->Compile2D(system);
}

bool cApplication::Create()
{
  std::cout<<"cApplication::Create"<<std::endl;
  voodoo::cImage image;

  {
    // Get a screenshot
    cScreen screen;
    size_t monitor = screen.GetMonitorContainingCursor();

    spitfire::math::cRectangle rect;
    size_t colorDepthBits = 32;
    screen.GetMonitorProperties(monitor, rect, colorDepthBits);

    const size_t width = rect.GetWidth();
    const size_t height = rect.GetHeight();
    std::vector<uint8_t> buffer;
    buffer.resize(width * height * 4, 0);
    image.CreateFromBuffer(buffer.data(), width, height, opengl::PIXELFORMAT::R8G8B8A8);

    screen.GetScreenShotOfMonitor(monitor, image);
  }

  // Create a power of two image from our screenshot
  const size_t imageWidth = image.GetWidth();
  const size_t imageHeight = image.GetHeight();
  // TODO: For dual monitors this width and height is incorrect
  //const size_t textureWidth = spitfire::math::NextPowerOfTwo(max(imageWidth, imageHeight));
  const size_t textureWidth = spitfire::math::NextPowerOfTwo(max(1280, 1280));
  const size_t textureHeight = textureWidth;
  std::vector<uint8_t> buffer;
  buffer.resize(textureWidth * textureHeight * 4, 0);

  const uint8_t* pData = image.GetPointerToBuffer();
  for (size_t y = 0; y < imageHeight; y++) {
    for (size_t x = 0; x < imageWidth; x++) {
      const size_t src = ((y * imageWidth) + x) * 4;
      const size_t dest = ((y * textureWidth) + x) * 4;
      buffer[dest] = pData[src];
      buffer[dest + 1] = pData[src + 1];
      buffer[dest + 2] = pData[src + 2];
      buffer[dest + 3] = pData[src + 3];
    }
  }

  voodoo::cImage imagePowerOfTwo;
  imagePowerOfTwo.CreateFromBuffer(buffer.data(), textureWidth, textureHeight, opengl::PIXELFORMAT::R8G8B8A8);


  // Create our window
  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  opengl::cResolution resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 1024) || (resolution.height < 768) || (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8)) {
    std::cout<<"cApplication::Create Current screen resolution is not adequate "<<resolution.width<<"x"<<resolution.height<<std::endl;
    return false;
  }

  // We will use the same resolution and depth in fullscreen
  resolution.width = 1280;
  resolution.height = 1024;
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;
  const bool bIsFullScreen = true;

  // For testing in windowed mode
  //resolution.width = 1024;
  //resolution.height = 768;
  //resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;
  //const bool bIsFullScreen = false;

  std::cout<<"cApplication::Create Calling create window"<<std::endl;
  pWindow = system.CreateWindow(TEXT("openglmm_heightmap"), resolution, bIsFullScreen);
  if (pWindow == nullptr) {
    std::cout<<"cApplication::Create Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }

  pShaderScreenQuad = pContext->CreateShader(TEXT("shaders/passthrough.vert"), TEXT("shaders/passthrough.frag"));
  assert(pShaderScreenQuad != nullptr);

  pStaticVertexBufferObjectScreenQuad = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);
  CreateScreenQuadVBO();


  // Create our screenshot texture
  pTextureScreenShot = pContext->CreateTextureFromImage(imagePowerOfTwo);
  if (pTextureScreenShot == nullptr) {
    std::cout<<"cApplication::Create CreateTextureFromImage FAILED to create texture from desktop image, returning false"<<std::endl;
    return false;
  }

  pStaticVertexBufferObjectScreenBlendQuad = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectScreenBlendQuad != nullptr);
  const float fRatioOfTextureWidthToScreenShotWidth = float(1280) / float(textureWidth);
  const float fRatioOfTextureHeightToScreenShotHeight = float(1024) / float(textureHeight);
  CreateScreenBlendQuadVBO(fRatioOfTextureWidthToScreenShotWidth, fRatioOfTextureHeightToScreenShotHeight);

  pShaderScreenBlendQuad = pContext->CreateShader(TEXT("shaders/blend.vert"), TEXT("shaders/blend.frag"));
  assert(pShaderScreenBlendQuad != nullptr);



  // Render one frame of the screenshot
  {
    assert(pContext != nullptr);
    assert(pContext->IsValid());
    assert(pTextureScreenShot != nullptr);
    assert(pTextureScreenShot->IsValid());

    assert(pShaderScreenBlendQuad != nullptr);
    assert(pShaderScreenBlendQuad->IsCompiledProgram());
    assert(pStaticVertexBufferObjectScreenBlendQuad != nullptr);
    assert(pStaticVertexBufferObjectScreenBlendQuad->IsCompiled());

    uint32_t T0 = 0;
    uint32_t Frames = 0;

    //uint32_t currentTime = SDL_GetTicks();

    // Setup mouse
    pWindow->ShowCursor(false);
    pWindow->WarpCursorToMiddleOfScreen();


    // Update window events
    pWindow->UpdateEvents();

    // Update state
    //currentTime = SDL_GetTicks();

    const float fBlend0 = 1.0f;
    const float fBlend1 = 0.0f;

    {
      // Render the screenshot
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRendering();

      // Now draw an overlay of our texture
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      // Move the quad into the bottom right hand corner of the screen
      spitfire::math::cMat4 matModelView2D;
      matModelView2D.SetTranslation(0.5f, 0.5f, 0.0f);

      pContext->BindTexture(0, *pTextureScreenShot);
      pContext->BindTexture(1, *pTextureScreenShot);

      pContext->BindShader(*pShaderScreenBlendQuad);

      pContext->SetShaderConstant("fBlend0", fBlend0);
      pContext->SetShaderConstant("fBlend1", fBlend1);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenBlendQuad);

      {
        pContext->SetModelViewMatrix(matModelView2D);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectScreenBlendQuad);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenBlendQuad);

      pContext->UnBindShader(*pShaderScreenBlendQuad);

      pContext->UnBindTexture(1, *pTextureScreenShot);
      pContext->UnBindTexture(0, *pTextureScreenShot);

      pContext->EndRenderMode2D();

      pContext->EndRendering();
    }

    // Gather our frames per second
    Frames++;
    {
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 5000) {
        float seconds = (t - T0) / 1000.0;
        float fps = Frames / seconds;
        std::cout<<Frames<<" frames in "<<seconds<<" seconds = "<<fps<<" FPS"<<std::endl;
        T0 = t;
        Frames = 0;
      }
    }
  }


  // Now we can continue loading the heightmap

  std::cout<<"cApplication::Create Creating frame buffer object"<<std::endl;
  pTextureFrameBufferObject = pContext->CreateTextureFrameBufferObject(1024, 1024, opengl::PIXELFORMAT::R8G8B8A8);
  assert(pTextureFrameBufferObject != nullptr);


  std::cout<<"cApplication::Create Creating diffuse and detail textures"<<std::endl;
  pTextureDiffuse = pContext->CreateTexture(TEXT("textures/diffuse.png"));
  pTextureDetail = pContext->CreateTexture(TEXT("textures/detail.png"));
  pTextureLightMap = pContext->CreateTexture(TEXT("textures/lightmap.png"));

  pShaderHeightmap = pContext->CreateShader(TEXT("shaders/heightmap.vert"), TEXT("shaders/heightmap.frag"));

  cHeightmapData data(TEXT("textures/heightmap.png"));

  width = data.GetWidth();
  depth = data.GetDepth();

  scale.Set(0.2f, 0.2f, 10.0f);

  pStaticVertexBufferObjectHeightmapQuads = pContext->CreateStaticVertexBufferObject();
  CreateHeightmapQuads(pStaticVertexBufferObjectHeightmapQuads, data, scale);

  pStaticVertexBufferObjectHeightmapQuadsIndexed = pContext->CreateStaticVertexBufferObject();
  CreateHeightmapQuadsIndexed(pStaticVertexBufferObjectHeightmapQuadsIndexed, data, scale);


  pShaderMask = pContext->CreateShader(TEXT("shaders/grass.vert"), TEXT("shaders/grass.frag"));


  pShaderPassThroughWithColour = pContext->CreateShader(TEXT("shaders/passthroughwithcolour.vert"), TEXT("shaders/passthroughwithcolour.frag"));
  assert(pShaderPassThroughWithColour != nullptr);


  grass.pTexture = pContext->CreateTexture(TEXT("textures/grass.png"));

  grass.pVBO = pContext->CreateStaticVertexBufferObject();
  assert(grass.pVBO != nullptr);

  flowers.pTexture = pContext->CreateTexture(TEXT("textures/flowers.png"));

  flowers.pVBO = pContext->CreateStaticVertexBufferObject();
  assert(flowers.pVBO != nullptr);

  trees.pTexture = pContext->CreateTexture(TEXT("textures/tree.png"));

  trees.pVBO = pContext->CreateStaticVertexBufferObject();
  assert(trees.pVBO != nullptr);

  rocks.pTexture = pContext->CreateTexture(TEXT("textures/rock.png"));

  rocks.pVBO = pContext->CreateStaticVertexBufferObject();
  assert(rocks.pVBO != nullptr);


  CreateVegetation(data, scale);


  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);

  return true;
}

void cApplication::Destroy()
{
  if (pStaticVertexBufferObjectScreenQuad != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectScreenQuad);
    pStaticVertexBufferObjectScreenQuad = nullptr;
  }
  if (pStaticVertexBufferObjectScreenBlendQuad != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectScreenBlendQuad);
    pStaticVertexBufferObjectScreenBlendQuad = nullptr;
  }

  if (pShaderScreenQuad != nullptr) {
    pContext->DestroyShader(pShaderScreenQuad);
    pShaderScreenQuad = nullptr;
  }
  if (pShaderScreenBlendQuad != nullptr) {
    pContext->DestroyShader(pShaderScreenBlendQuad);
    pShaderScreenBlendQuad = nullptr;
  }
  if (pTextureScreenShot != nullptr) {
    pContext->DestroyTexture(pTextureScreenShot);
    pTextureScreenShot = nullptr;
  }


  if (rocks.pVBO != nullptr) {
    pContext->DestroyStaticVertexBufferObject(rocks.pVBO);
    rocks.pVBO = nullptr;
  }
  if (rocks.pTexture != nullptr) {
    pContext->DestroyTexture(rocks.pTexture);
    rocks.pTexture = nullptr;
  }

  if (trees.pVBO != nullptr) {
    pContext->DestroyStaticVertexBufferObject(trees.pVBO);
    trees.pVBO = nullptr;
  }
  if (trees.pTexture != nullptr) {
    pContext->DestroyTexture(trees.pTexture);
    trees.pTexture = nullptr;
  }

  if (flowers.pVBO != nullptr) {
    pContext->DestroyStaticVertexBufferObject(flowers.pVBO);
    flowers.pVBO = nullptr;
  }
  if (flowers.pTexture != nullptr) {
    pContext->DestroyTexture(flowers.pTexture);
    flowers.pTexture = nullptr;
  }

  if (grass.pVBO != nullptr) {
    pContext->DestroyStaticVertexBufferObject(grass.pVBO);
    grass.pVBO = nullptr;
  }
  if (grass.pTexture != nullptr) {
    pContext->DestroyTexture(grass.pTexture);
    grass.pTexture = nullptr;
  }


  if (pShaderPassThroughWithColour != nullptr) {
    pContext->DestroyShader(pShaderPassThroughWithColour);
    pShaderPassThroughWithColour = nullptr;
  }
  if (pShaderMask != nullptr) {
    pContext->DestroyShader(pShaderMask);
    pShaderMask = nullptr;
  }



  if (pStaticVertexBufferObjectHeightmapQuadsIndexed != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectHeightmapQuadsIndexed);
    pStaticVertexBufferObjectHeightmapQuadsIndexed = nullptr;
  }
  if (pStaticVertexBufferObjectHeightmapQuads != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectHeightmapQuads);
    pStaticVertexBufferObjectHeightmapQuads = nullptr;
  }

  if (pShaderHeightmap != nullptr) {
    pContext->DestroyShader(pShaderHeightmap);
    pShaderHeightmap = nullptr;
  }

  if (pTextureDetail != nullptr) {
    pContext->DestroyTexture(pTextureDetail);
    pTextureDetail = nullptr;
  }
  if (pTextureLightMap != nullptr) {
    pContext->DestroyTexture(pTextureLightMap);
    pTextureLightMap = nullptr;
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

  if (!bIsMouseMovement) return;

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
  } else if (event.IsButtonDown()) {
    const float fZoomIncrement = 5.0f;

    switch (event.GetButton()) {
      case SDL_BUTTON_WHEELUP: {
        std::cout<<"cApplication::_OnMouseEvent Wheel up"<<std::endl;
        fZoom -= fZoomIncrement;
        break;
      }
      case SDL_BUTTON_WHEELDOWN: {
        std::cout<<"cApplication::_OnMouseEvent Wheel down"<<std::endl;
        fZoom += fZoomIncrement;
        break;
      }
    };
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
    }
  } else if (event.IsKeyUp()) {
    switch (event.GetKeyCode()) {
      case SDLK_1: {
        std::cout<<"cApplication::_OnKeyboardEvent Switching to non-indexed quads"<<std::endl;
        bUseQuadsIndexed = false;
        break;
      }
      case SDLK_2: {
        std::cout<<"cApplication::_OnKeyboardEvent Switching to indexed quads"<<std::endl;
        bUseQuadsIndexed = true;
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

void cApplication::Run()
{
  std::cout<<"cApplication::Run"<<std::endl;

  assert(pContext != nullptr);
  assert(pContext->IsValid());
  assert(pTextureScreenShot != nullptr);
  assert(pTextureScreenShot->IsValid());
  assert(pShaderMask != nullptr);
  assert(pShaderMask->IsCompiledProgram());
  assert(pShaderPassThroughWithColour != nullptr);
  assert(pShaderPassThroughWithColour->IsCompiledProgram());

  assert(grass.pTexture != nullptr);
  assert(grass.pTexture->IsValid());
  assert(grass.pVBO != nullptr);
  assert(grass.pVBO->IsCompiled());

  assert(flowers.pTexture != nullptr);
  assert(flowers.pTexture->IsValid());
  assert(flowers.pVBO != nullptr);
  //assert(flowers.pVBO->IsCompiled());

  assert(trees.pTexture != nullptr);
  assert(trees.pTexture->IsValid());
  assert(trees.pVBO != nullptr);
  assert(trees.pVBO->IsCompiled());

  assert(rocks.pTexture != nullptr);
  assert(rocks.pTexture->IsValid());
  assert(rocks.pVBO != nullptr);
  assert(rocks.pVBO->IsCompiled());

  assert(pTextureDiffuse != nullptr);
  assert(pTextureDiffuse->IsValid());
  assert(pTextureLightMap != nullptr);
  assert(pTextureLightMap->IsValid());
  assert(pTextureDetail != nullptr);
  assert(pTextureDetail->IsValid());
  assert(pShaderHeightmap != nullptr);
  assert(pShaderHeightmap->IsCompiledProgram());

  assert(pStaticVertexBufferObjectHeightmapQuads != nullptr);
  assert(pStaticVertexBufferObjectHeightmapQuads->IsCompiled());
  assert(pStaticVertexBufferObjectHeightmapQuadsIndexed != nullptr);
  assert(pStaticVertexBufferObjectHeightmapQuadsIndexed->IsCompiled());

  assert(pShaderScreenBlendQuad != nullptr);
  assert(pShaderScreenBlendQuad->IsCompiledProgram());
  assert(pStaticVertexBufferObjectScreenBlendQuad != nullptr);
  assert(pStaticVertexBufferObjectScreenBlendQuad->IsCompiled());

  assert(pShaderScreenQuad != nullptr);
  assert(pShaderScreenQuad->IsCompiledProgram());
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);
  assert(pStaticVertexBufferObjectScreenQuad->IsCompiled());

  const spitfire::math::cColour sunColour(0.2, 0.2, 0.0);

  // Center the camera at the middle of the heightmap
  spitfire::math::cMat4 matTranslation;
  matTranslation.SetTranslation(-0.5f * scale.x * float(width), -0.5f * scale.y * float(depth), 0.0f);

  // Set the defaults for the orbiting camera
  {
    const spitfire::math::cVec3 axisZ(0.0f, 0.0f, 1.0f);
    rotationZ.SetFromAxisAngleDegrees(axisZ, 0.0f);

    const spitfire::math::cVec3 axisX(1.0f, 0.0f, 0.0f);
    rotationX.SetFromAxisAngleDegrees(axisX, -20.0f);
  }
  fZoom = 60.0f;


  spitfire::math::cMat4 matRotation;

  uint32_t T0 = 0;
  uint32_t Frames = 0;

  //uint32_t previousTime = SDL_GetTicks();
  uint32_t currentTime = SDL_GetTicks();

  // Setup mouse
  pWindow->ShowCursor(false);
  pWindow->WarpCursorToMiddleOfScreen();


  uint32_t duration = (5 * 1000); // End in 5 seconds time

  uint32_t startTime = SDL_GetTicks();
  uint32_t endTime = startTime + duration;

  while (!bIsDone && (currentTime < endTime)) {
    // Update window events
    pWindow->UpdateEvents();

    // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
    pWindow->WarpCursorToMiddleOfScreen();

    // Update state
    //previousTime = currentTime;
    currentTime = SDL_GetTicks();

    const float fComplete0To1 = min(spitfire::math::square((float(currentTime) - float(startTime)) / float(duration)), 1.0f);
    const float fBlend0 = 1.0f - fComplete0To1;
    const float fBlend1 = fComplete0To1;

    // If we are more than 3/4 of the way through fading then we can allow the user to look around
    if (fComplete0To1 > 0.75f) bIsMouseMovement = true;

    matRotation.SetRotation(rotationZ * rotationX);



    {
      // Render the scene into the frame buffer object
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObject);

      if (bIsWireframe) pContext->EnableWireframe();

      const spitfire::math::cVec3 offset = matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, -fZoom, 0.0f));
      const spitfire::math::cVec3 up = matRotation.GetRotatedVec3(spitfire::math::v3Up);

      const spitfire::math::cVec3 target(0.0f, 0.0f, 0.0f);
      const spitfire::math::cVec3 eye(target + offset);
      spitfire::math::cMat4 matModelView;
      matModelView.LookAt(eye, target, up);


      // Render terrain
      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureLightMap);
      pContext->BindTexture(2, *pTextureDetail);

      pContext->BindShader(*pShaderHeightmap);

      if (!bUseQuadsIndexed) {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuads);
          pContext->SetModelViewMatrix(matModelView * matTranslation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectHeightmapQuads);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuads);
      } else {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuadsIndexed);
          pContext->SetModelViewMatrix(matModelView * matTranslation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectHeightmapQuadsIndexed);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuadsIndexed);
      }

      pContext->UnBindShader(*pShaderHeightmap);

      pContext->UnBindTexture(2, *pTextureDetail);
      pContext->UnBindTexture(1, *pTextureLightMap);
      pContext->UnBindTexture(0, *pTextureDiffuse);


      // Render grass
      pContext->BindTexture(0, *grass.pTexture);

      pContext->BindShader(*pShaderMask);

      pContext->BindStaticVertexBufferObject(*grass.pVBO);
        pContext->SetModelViewMatrix(matModelView * matTranslation);
        pContext->DrawStaticVertexBufferObjectQuads(*grass.pVBO);
      pContext->UnBindStaticVertexBufferObject(*grass.pVBO);

      pContext->UnBindShader(*pShaderMask);

      pContext->UnBindTexture(0, *grass.pTexture);


      // Render trees
      pContext->BindTexture(0, *trees.pTexture);

      pContext->BindShader(*pShaderMask);

      pContext->BindStaticVertexBufferObject(*trees.pVBO);
        pContext->SetModelViewMatrix(matModelView * matTranslation);
        pContext->DrawStaticVertexBufferObjectQuads(*trees.pVBO);
      pContext->UnBindStaticVertexBufferObject(*trees.pVBO);

      pContext->UnBindShader(*pShaderMask);

      pContext->UnBindTexture(0, *trees.pTexture);


      // Render rocks
      pContext->BindTexture(0, *rocks.pTexture);

      pContext->BindShader(*pShaderPassThroughWithColour);

      pContext->BindStaticVertexBufferObject(*rocks.pVBO);
        pContext->SetModelViewMatrix(matModelView * matTranslation);
        pContext->DrawStaticVertexBufferObjectQuads(*rocks.pVBO);
      pContext->UnBindStaticVertexBufferObject(*rocks.pVBO);

      pContext->UnBindShader(*pShaderPassThroughWithColour);

      pContext->UnBindTexture(0, *rocks.pTexture);


      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToTexture(*pTextureFrameBufferObject);
    }

    {
      // Render the scene with the new texture
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRendering();

      // Now draw an overlay of our rendered texture
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      // Move the quad into the bottom right hand corner of the screen
      spitfire::math::cMat4 matModelView2D;
      matModelView2D.SetTranslation(0.5f, 0.5f, 0.0f);

      pContext->BindTexture(0, *pTextureScreenShot);
      pContext->BindTexture(1, *pTextureFrameBufferObject);

      pContext->BindShader(*pShaderScreenBlendQuad);

      pContext->SetShaderConstant("fBlend0", fBlend0);
      pContext->SetShaderConstant("fBlend1", fBlend1);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenBlendQuad);

      {
        pContext->SetModelViewMatrix(matModelView2D);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectScreenBlendQuad);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenBlendQuad);

      pContext->UnBindShader(*pShaderScreenBlendQuad);

      pContext->UnBindTexture(1, *pTextureFrameBufferObject);
      pContext->UnBindTexture(0, *pTextureScreenShot);

      pContext->EndRenderMode2D();

      pContext->EndRendering();
    }

    // Gather our frames per second
    Frames++;
    {
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 5000) {
        float seconds = (t - T0) / 1000.0;
        float fps = Frames / seconds;
        std::cout<<Frames<<" frames in "<<seconds<<" seconds = "<<fps<<" FPS"<<std::endl;
        T0 = t;
        Frames = 0;
      }
    }
  };

  bIsMouseMovement = true;

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();

    // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
    pWindow->WarpCursorToMiddleOfScreen();

    // Update state
    //previousTime = currentTime;
    currentTime = SDL_GetTicks();

    matRotation.SetRotation(rotationZ * rotationX);


    {
      // Render the scene into the frame buffer object
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObject);

      if (bIsWireframe) pContext->EnableWireframe();

      const spitfire::math::cVec3 offset = matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, -fZoom, 0.0f));
      const spitfire::math::cVec3 up = matRotation.GetRotatedVec3(spitfire::math::v3Up);

      const spitfire::math::cVec3 target(0.0f, 0.0f, 0.0f);
      const spitfire::math::cVec3 eye(target + offset);
      spitfire::math::cMat4 matModelView;
      matModelView.LookAt(eye, target, up);


      // Render terrain
      pContext->BindTexture(0, *pTextureDiffuse);
      pContext->BindTexture(1, *pTextureLightMap);
      pContext->BindTexture(2, *pTextureDetail);

      pContext->BindShader(*pShaderHeightmap);

      if (!bUseQuadsIndexed) {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuads);
          pContext->SetModelViewMatrix(matModelView * matTranslation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectHeightmapQuads);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuads);
      } else {
        pContext->BindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuadsIndexed);
          pContext->SetModelViewMatrix(matModelView * matTranslation);
          pContext->DrawStaticVertexBufferObjectQuads(*pStaticVertexBufferObjectHeightmapQuadsIndexed);
        pContext->UnBindStaticVertexBufferObject(*pStaticVertexBufferObjectHeightmapQuadsIndexed);
      }

      pContext->UnBindShader(*pShaderHeightmap);

      pContext->UnBindTexture(2, *pTextureDetail);
      pContext->UnBindTexture(1, *pTextureLightMap);
      pContext->UnBindTexture(0, *pTextureDiffuse);


      // Render grass
      pContext->BindTexture(0, *grass.pTexture);

      pContext->BindShader(*pShaderMask);

      pContext->BindStaticVertexBufferObject(*grass.pVBO);
        pContext->SetModelViewMatrix(matModelView * matTranslation);
        pContext->DrawStaticVertexBufferObjectQuads(*grass.pVBO);
      pContext->UnBindStaticVertexBufferObject(*grass.pVBO);

      pContext->UnBindShader(*pShaderMask);

      pContext->UnBindTexture(0, *grass.pTexture);


      // Render trees
      pContext->BindTexture(0, *trees.pTexture);

      pContext->BindShader(*pShaderMask);

      pContext->BindStaticVertexBufferObject(*trees.pVBO);
        pContext->SetModelViewMatrix(matModelView * matTranslation);
        pContext->DrawStaticVertexBufferObjectQuads(*trees.pVBO);
      pContext->UnBindStaticVertexBufferObject(*trees.pVBO);

      pContext->UnBindShader(*pShaderMask);

      pContext->UnBindTexture(0, *trees.pTexture);


      // Render rocks
      pContext->BindTexture(0, *rocks.pTexture);

      pContext->BindShader(*pShaderPassThroughWithColour);

      pContext->BindStaticVertexBufferObject(*rocks.pVBO);
        pContext->SetModelViewMatrix(matModelView * matTranslation);
        pContext->DrawStaticVertexBufferObjectQuads(*rocks.pVBO);
      pContext->UnBindStaticVertexBufferObject(*rocks.pVBO);

      pContext->UnBindShader(*pShaderPassThroughWithColour);

      pContext->UnBindTexture(0, *rocks.pTexture);


      if (bIsWireframe) pContext->DisableWireframe();

      pContext->EndRenderToTexture(*pTextureFrameBufferObject);
    }

    {
      // Render the scene with the new texture
      const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRendering();

      // Now draw an overlay of our rendered texture
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      // Move the quad into the bottom right hand corner of the screen
      spitfire::math::cMat4 matModelView2D;
      matModelView2D.SetTranslation(0.5f, 0.5f, 0.0f);

      pContext->BindTexture(0, *pTextureFrameBufferObject);

      pContext->BindShader(*pShaderScreenQuad);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenQuad);

      {
        pContext->SetModelViewMatrix(matModelView2D);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectScreenQuad);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenQuad);

      pContext->UnBindShader(*pShaderScreenQuad);

      pContext->UnBindTexture(0, *pTextureFrameBufferObject);

      pContext->EndRenderMode2D();

      pContext->EndRendering();
    }

    // Gather our frames per second
    Frames++;
    {
      uint32_t t = SDL_GetTicks();
      if (t - T0 >= 5000) {
        float seconds = (t - T0) / 1000.0;
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

  cApplication application;

  bIsSuccess = application.Create();
  if (bIsSuccess) application.Run();

  application.Destroy();

  return bIsSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}

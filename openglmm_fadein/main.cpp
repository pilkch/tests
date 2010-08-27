#include <cassert>
#include <cmath>
#include <cstring>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>

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

  bool GetScreenShotOfMonitor(size_t monitor, opengl::cImage& image) const;
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

bool cScreen::GetScreenShotOfMonitor(size_t monitor, opengl::cImage& image) const
{
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

  for (size_t i = 0; (i < height); i++) {
    for (size_t j = 0; (j < width); j++) {
      int index = ((i * width) + j) * 4;

      bool bIsFlip = !(
        (i > height / 4 - 1) && (i < 3 * height / 4) &&
        (j > width / 4 - 1)  && (j < 3 * width / 4)
      );

      size_t pixel = XGetPixel(pImage, j, i);
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

  std::cout<<"cScreen::GetScreenShotOfMonitor XGetImage returning true"<<std::endl;
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

  size_t GetLightmapWidth() const { return widthLightmap; }
  size_t GetLightmapDepth() const { return depthLightmap; }
  const uint8_t* GetLightmapBuffer() const;

private:
  spitfire::math::cVec3 GetNormalOfTriangle(const spitfire::math::cVec3& p0, const spitfire::math::cVec3& p1, const spitfire::math::cVec3& p2) const;

  static void GenerateLightmap(const std::vector<float>& heightmap, std::vector<spitfire::math::cColour>& lightmap, float fScaleZ, const spitfire::math::cColour& ambientColour, const spitfire::math::cColour& shadowColor, int size, float lightDir[3]);

  static void DoubleImageSize(const std::vector<spitfire::math::cColour>& source, size_t width, size_t height, std::vector<spitfire::math::cColour>& destination);
  void SmoothImage(const std::vector<spitfire::math::cColour>& source, size_t width, size_t height, size_t iterations, std::vector<spitfire::math::cColour>& destination) const;

  spitfire::math::cColour GetLightmapPixel(const std::vector<spitfire::math::cColour>& lightmap, size_t x, size_t y) const;

  std::vector<float> heightmap;
  size_t width;
  size_t depth;

  std::vector<uint8_t> lightmap;
  size_t widthLightmap;
  size_t depthLightmap;
};

cHeightmapData::cHeightmapData(const opengl::string_t& sFilename)
{
  opengl::cImage image;
  if (!image.LoadFromFile(sFilename)) {
    std::cout<<"cHeightmapData::cHeightmapData Could not load "<<opengl::string::ToUTF8(sFilename)<<std::endl;
    assert(false);
  }

  width = image.GetWidth();
  depth = image.GetHeight();

  // Create heightmap data
  const size_t n = width * depth;

  heightmap.resize(n, 0);

  const uint8_t* pPixels = image.GetPointerToSurfacePixelBuffer();

  for (size_t y = 0; y < depth; y++) {
    for (size_t x = 0; x < width; x++) {
      heightmap[(y * width) + x] = float(pPixels[(y * width) + x]) / 255.0f;
    }
  }


  // Calculate shadowmap texture
  std::vector<spitfire::math::cColour> _lightmap;
  _lightmap.resize(n, spitfire::math::cColour());

  // I find that an exagerated z scale gives a better, more obvious result
  const float fScaleZ = 100.0f;
  const spitfire::math::cColour ambientColour(1.0f, 1.0f, 1.0f);
  const spitfire::math::cColour shadowColour = 0.5f * ambientColour;
  int size = width;
  const spitfire::math::cVec3 sun(-50.0f, -100.0f, 1.0f);
  const spitfire::math::cVec3 origin(0.0f, 0.0f, 0.0f);
  const spitfire::math::cVec3 dir = (origin - sun).GetNormalised();
  float lightDir[3] = { dir.x, dir.y, dir.z };

  GenerateLightmap(heightmap, _lightmap, fScaleZ, ambientColour, shadowColour, size, lightDir);


  widthLightmap = width;
  depthLightmap = depth;

  {
    // Smooth the lightmap
    std::vector<spitfire::math::cColour> smoothed;

    SmoothImage(_lightmap, widthLightmap, depthLightmap, 10, smoothed);

    _lightmap = smoothed;
  }


  for (size_t i = 0; i < 3; i++) {
    // Double the resolution of the lightmap
    std::vector<spitfire::math::cColour> lightmapCopy;

    DoubleImageSize(_lightmap, widthLightmap, depthLightmap, lightmapCopy);

    _lightmap = lightmapCopy;
    widthLightmap *= 2;
    depthLightmap *= 2;


    // Smooth the lightmap
    std::vector<spitfire::math::cColour> smoothed;

    SmoothImage(_lightmap, widthLightmap, depthLightmap, 10, smoothed);

    _lightmap = smoothed;
  }

  const size_t nLightmap = widthLightmap * depthLightmap;


  // Copy the lightmap from the cColour vector to the uint8_t vector
  lightmap.resize(nLightmap * 4, 0);

  for (size_t y = 0; y < depthLightmap; y++) {
    for (size_t x = 0; x < widthLightmap; x++) {
      const size_t src = (y * widthLightmap) + x;
      const size_t dst = 4 * ((y * widthLightmap) + x);
      lightmap[dst + 0] = uint8_t(_lightmap[src].r * 255.0f);
      lightmap[dst + 1] = uint8_t(_lightmap[src].g * 255.0f);
      lightmap[dst + 2] = uint8_t(_lightmap[src].b * 255.0f);
      lightmap[dst + 3] = uint8_t(_lightmap[src].a * 255.0f);
    }
  }
}

void cHeightmapData::DoubleImageSize(const std::vector<spitfire::math::cColour>& source, size_t widthSource, size_t heightSource, std::vector<spitfire::math::cColour>& destination)
{
  const size_t widthDestination = 2 * widthSource;
  const size_t heightDestination = 2 * heightSource;
  const size_t n = widthDestination * heightDestination;

  destination.resize(n, spitfire::math::cColour());

  for (size_t y = 0; y < heightSource; y++) {
    for (size_t x = 0; x < widthSource; x++) {
      const spitfire::math::cColour& colour = source[(y * widthSource) + x];
      destination[(2 * ((y * widthDestination) + x)) + 0] = colour;
      destination[(2 * ((y * widthDestination) + x)) + 1] = colour;
      destination[widthDestination + (2 * ((y * widthDestination) + x)) + 0] = colour;
      destination[widthDestination + (2 * ((y * widthDestination) + x)) + 1] = colour;
    }
  }
}

void cHeightmapData::SmoothImage(const std::vector<spitfire::math::cColour>& source, size_t width, size_t height, size_t iterations, std::vector<spitfire::math::cColour>& destination) const
{
  const size_t n = width * height;

  destination.resize(n, spitfire::math::cColour());

  std::vector<spitfire::math::cColour> temp = source;

  spitfire::math::cColour surrounding[5];

  for (size_t i = 0; i < iterations; i++) {
    for (size_t y = 0; y < depthLightmap; y++) {
      for (size_t x = 0; x < widthLightmap; x++) {

        surrounding[0] = surrounding[1] = surrounding[2] = surrounding[3] = surrounding[4] = GetLightmapPixel(temp, x, y);

        // We sample from the 4 surrounding pixels in a cross shape
        if (x != 0) surrounding[0] = GetLightmapPixel(temp, x - 1, y);
        if ((y + 1) < depthLightmap) surrounding[1] = GetLightmapPixel(temp, x, y + 1);
        if (y != 0) surrounding[3] = GetLightmapPixel(temp, x, y - 1);
        if ((x + 1) < widthLightmap) surrounding[4] = GetLightmapPixel(temp, x + 1, y);

        //const spitfire::math::cColour averageOfSurrounding = 0.25f * (surrounding[0] + surrounding[1] + surrounding[3] + surrounding[4]);
        //const spitfire::math::cColour final = 0.5f * (surrounding[2] + averageOfSurrounding);
        const spitfire::math::cColour final = 0.25f * (surrounding[0] + surrounding[1] + surrounding[3] + surrounding[4]);

        const size_t index = (y * widthLightmap) + x;

        destination[index] = final;
      }
    }


    // If we are still going then set temp to destination for the next iteration
    if ((i + 1) < iterations) temp = destination;
  }
}

spitfire::math::cColour cHeightmapData::GetLightmapPixel(const std::vector<spitfire::math::cColour>& lightmap, size_t x, size_t y) const
{
  assert(x < widthLightmap);
  assert(y < depthLightmap);

  const size_t index = (y * widthLightmap) + x;
  return lightmap[index];
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

const uint8_t* cHeightmapData::GetLightmapBuffer() const
{
  assert(!lightmap.empty());
  return &lightmap[0];
}


// http://gpwiki.org/index.php/Faster_Ray_Traced_Terrain_Shadow_Maps

void cHeightmapData::GenerateLightmap(const std::vector<float>& heightmap, std::vector<spitfire::math::cColour>& _lightmap, float fScaleZ, const spitfire::math::cColour& ambientColour, const spitfire::math::cColour& shadowColour, int size, float lightDir[3])
{
  assert(!_lightmap.empty());

  const size_t n = size * size;
  for (size_t index = 0; index < n; index++) {
    _lightmap[index] = ambientColour;
  }

  float px = 0.0f;
  float py = 0.0f;
  float height = 0.0f;
  float distance = 0.0f;
  int index = 0;

  // create flag buffer to indicate where we've been
  float* flagMap = new float[n];
  for (size_t i = 0; i < n; i++) flagMap[i] = 0;

  int* X = nullptr;
  int* Y = nullptr;
  int iX = 0;
  int iY = 0;
  int dirX = 0;
  int dirY = 0;

  // calculate absolute values for light direction
  float lightDirXMagnitude = lightDir[0];
  float lightDirZMagnitude = lightDir[2];
  if (lightDirXMagnitude < 0) lightDirXMagnitude *= -1;
  if (lightDirZMagnitude < 0) lightDirZMagnitude *= -1;

  // decide which loop will come first, the y loop or x loop
  // based on direction of light, makes calculations faster
  if (lightDirXMagnitude > lightDirZMagnitude) {
    Y = &iX;
    X = &iY;

    if(lightDir[0] < 0) {
      iY = size-1;
      dirY = -1;
    } else {
      iY = 0;
      dirY = 1;
    }

    if(lightDir[2] < 0) {
      iX = size-1;
      dirX = -1;
    } else {
      iX = 0;
      dirX = 1;
    }
  } else {
    Y = &iY;
    X = &iX;

    if(lightDir[0] < 0) {
      iX = size-1;
      dirX = -1;
    } else {
      iX = 0;
      dirX = 1;
    }

    if(lightDir[2] < 0) {
      iY = size-1;
      dirY = -1;
    } else {
      iY = 0;
      dirY = 1;
    }
  }

  // outer loop
  while (1) {
    // inner loop
    while (1) {
      // travel along the terrain until we:
      // (1) intersect another point
      // (2) find another point with previous collision data
      // (3) or reach the edge of the map
      px = *X;
      py = *Y;
      index = (*Y) * size + (*X);

      // travel along ray
      while (1) {
        px -= lightDir[0];
        py -= lightDir[2];

        // check if we've reached the boundary
        if (px < 0 || px >= size || py < 0 || py >= size) {
          flagMap[index] = -1;
          break;
        }

        // calculate interpolated values
        static int x0, x1, y0, y1;
        static float du, dv;
        static float interpolatedHeight, interpolatedFlagMap;
        static float heights[4];
        static float pixels[4];
        static float invdu, invdv;
        static float w0, w1, w2, w3;

        x0 = floor(px);
        x1 = ceil(px);
        y0 = floor(py);
        y1 = ceil(py);

        du = px - x0;
        dv = py - y0;

        invdu = 1.0 - du;
        invdv = 1.0 - dv;
        w0 = invdu * invdv;
        w1 = invdu * dv;
        w2 = du * invdv;
        w3 = du * dv;

        // compute interpolated height value from the heightmap direction below ray
        heights[0] = fScaleZ * heightmap[y0 * size + x0];
        heights[1] = fScaleZ * heightmap[y1 * size + x0];
        heights[2] = fScaleZ * heightmap[y0 * size + x1];
        heights[3] = fScaleZ * heightmap[y1 * size + x1];
        interpolatedHeight = w0 * heights[0] + w1 * heights[1] + w2 * heights[2] + w3 * heights[3];

        // compute interpolated flagmap value from point directly below ray
        pixels[0] = flagMap[y0 * size + x0];
        pixels[1] = flagMap[y1 * size + x0];
        pixels[2] = flagMap[y0 * size + x1];
        pixels[3] = flagMap[y1 * size + x1];
        interpolatedFlagMap = w0 * pixels[0] + w1 * pixels[1] + w2 * pixels[2] + w3 * pixels[3];

        // get distance from original point to current point
        distance = sqrt( (px - *X)*(px - *X) + (py - *Y) * (py - *Y) );

        // get height at current point while traveling along light ray
        height = (fScaleZ * heightmap[index]) + lightDir[1] * distance;

        // check intersection with either terrain or flagMap
        // if interpolatedHeight is less than interpolatedFlagMap that means we need to use the flagMap value instead
        // else use the height value
        static float val;
        val = interpolatedHeight;
        if(interpolatedHeight < interpolatedFlagMap) val = interpolatedFlagMap;
        if(height < val) {
          flagMap[index] = val - height;

          _lightmap[index] = shadowColour;

          break;
        }

        // check if pixel we've moved to is unshadowed
        // since the flagMap value we're using is interpolated, we will be in between shadowed and unshadowed areas
        // to compensate for this, simply define some epsilon value and use this as an offset from -1 to decide
        // if current point under the ray is unshadowed
        static float epsilon = 0.5f;
        if(interpolatedFlagMap < -1.0f+epsilon && interpolatedFlagMap > -1.0f-epsilon) {
          flagMap[index] = -1.0f;
          break;
        }
      }

      // update inner loop variable
      if(dirY < 0) {
        iY--;
        if(iY < 0) break;
      } else {
        iY++;
        if(iY >= size) break;
      }
    }

    // reset inner loop starting point
    if(dirY < 0) iY = size - 1;
    else iY = 0;

    // update outer loop variable
    if(dirX < 0) {
      iX--;
      if(iX < 0) break;
    } else {
      iX++;
      if(iX >= size) break;
    }
  }

  delete [] flagMap;
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

  void CreateScreenQuadVBO();

  void _OnWindowEvent(const opengl::cWindowEvent& event);
  void _OnMouseEvent(const opengl::cMouseEvent& event);
  void _OnKeyboardEvent(const opengl::cKeyboardEvent& event);

  bool bIsWireframe;
  bool bUseQuadsIndexed;
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
  opengl::cShader* pShaderScreenQuad;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectHeightmapQuads;
  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectHeightmapQuadsIndexed;

  opengl::cStaticVertexBufferObject* pStaticVertexBufferObjectScreenQuad;
};

cApplication::cApplication() :
  bIsWireframe(false),
  bUseQuadsIndexed(true),
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
  pShaderScreenQuad(nullptr),

  pStaticVertexBufferObjectHeightmapQuads(nullptr),
  pStaticVertexBufferObjectHeightmapQuadsIndexed(nullptr),

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
  opengl::cImage image;

  {
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

  const opengl::cCapabilities& capabilities = system.GetCapabilities();

  opengl::cResolution resolution = capabilities.GetCurrentResolution();
  if ((resolution.width < 1024) || (resolution.height < 768) || (resolution.pixelFormat != opengl::PIXELFORMAT::R8G8B8A8)) {
    std::cout<<"Current screen resolution is not adequate "<<resolution.width<<"x"<<resolution.height<<std::endl;
    return false;
  }

  // Set our required resolution
  resolution.width = 1024;
  resolution.height = 768;
  resolution.pixelFormat = opengl::PIXELFORMAT::R8G8B8A8;

  pWindow = system.CreateWindow(TEXT("openglmm_heightmap"), resolution, false);
  if (pWindow == nullptr) {
    std::cout<<"Window could not be created"<<std::endl;
    return false;
  }

  pContext = pWindow->GetContext();
  if (pContext == nullptr) {
    std::cout<<"Context could not be created"<<std::endl;
    return false;
  }

  pTextureFrameBufferObject = pContext->CreateTextureFrameBufferObject(512, 512, opengl::PIXELFORMAT::R8G8B8A8);
  assert(pTextureFrameBufferObject != nullptr);

  pTextureDiffuse = pContext->CreateTexture(TEXT("textures/diffuse.png"));
  pTextureDetail = pContext->CreateTexture(TEXT("textures/detail.png"));

  pShaderHeightmap = pContext->CreateShader(TEXT("shaders/heightmap.vert"), TEXT("shaders/heightmap.frag"));
  pShaderHeightmap->bTexUnit0 = true;
  pShaderHeightmap->bTexUnit1 = true;
  pShaderHeightmap->bTexUnit2 = true;

  cHeightmapData data(TEXT("textures/heightmap.png"));

  width = data.GetWidth();
  depth = data.GetDepth();

  scale.Set(0.2f, 0.2f, 10.0f);

  const uint8_t* pBuffer = data.GetLightmapBuffer();
  const size_t widthLightmap = data.GetLightmapWidth();
  const size_t depthLightmap = data.GetLightmapDepth();
  pTextureLightMap = pContext->CreateTextureFromBuffer(pBuffer, widthLightmap, depthLightmap, opengl::PIXELFORMAT::R8G8B8A8);


  pStaticVertexBufferObjectHeightmapQuads = pContext->CreateStaticVertexBufferObject();
  CreateHeightmapQuads(pStaticVertexBufferObjectHeightmapQuads, data, scale);

  pStaticVertexBufferObjectHeightmapQuadsIndexed = pContext->CreateStaticVertexBufferObject();
  CreateHeightmapQuadsIndexed(pStaticVertexBufferObjectHeightmapQuadsIndexed, data, scale);


  pShaderScreenQuad = pContext->CreateShader(TEXT("shaders/blend.vert"), TEXT("shaders/blend.frag"));
  assert(pShaderScreenQuad != nullptr);
  pShaderScreenQuad->bTexUnit0 = true;

  pStaticVertexBufferObjectScreenQuad = pContext->CreateStaticVertexBufferObject();
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);
  CreateScreenQuadVBO();


  // Setup our event listeners
  pWindow->SetWindowEventListener(*this);
  pWindow->SetInputEventListener(*this);


  const size_t imageWidth = image.GetWidth();
  const size_t imageHeight = image.GetHeight();
  const size_t width = 1024;
  const size_t height = 1024;
  std::vector<uint8_t> buffer;
  buffer.resize(width * height * 4, 0);

  const uint8_t* pData = image.GetPointerToData();
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      const size_t src = ((y * imageWidth) + x) * 4;
      const size_t dest = ((y * imageHeight) + x) * 4;
      buffer[dest] = pData[src];
      buffer[dest + 1] = pData[src + 1];
      buffer[dest + 2] = pData[src + 2];
      buffer[dest + 3] = pData[src + 3];
    }
  }

  opengl::cImage imagePowerOfTwo;
  imagePowerOfTwo.CreateFromBuffer(buffer.data(), width, height, opengl::PIXELFORMAT::R8G8B8A8);

  pTextureScreenShot = pContext->CreateTextureFromImage(imagePowerOfTwo);
  if (pTextureScreenShot == nullptr) {
    std::cout<<"cApplication::Create CreateTextureFromImage FAILED to create texture from desktop image, returning false"<<std::endl;
    return false;
  }

  return true;
}

void cApplication::Destroy()
{
  if (pStaticVertexBufferObjectScreenQuad != nullptr) {
    pContext->DestroyStaticVertexBufferObject(pStaticVertexBufferObjectScreenQuad);
    pStaticVertexBufferObjectScreenQuad = nullptr;
  }

  if (pShaderScreenQuad != nullptr) {
    pContext->DestroyShader(pShaderScreenQuad);
    pShaderScreenQuad = nullptr;
  }
  if (pTextureScreenShot != nullptr) {
    pContext->DestroyTexture(pTextureScreenShot);
    pTextureScreenShot = nullptr;
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
    float fNewAngleDegrees = rotationZ.GetAngleDegrees();
    //std::cout<<"cApplication::_OnMouseEvent z newAxis={ "<<newAxis.x<<", "<<newAxis.y<<", "<<newAxis.z<<" } angle="<<fNewAngleDegrees<<std::endl;

    newAxis = rotationX.GetAxis();
    fNewAngleDegrees = rotationX.GetAngleDegrees();
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
        std::cout<<"cApplication::_OnKeyboardEvent F1 key pressed, quiting"<<std::endl;
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
  assert(pContext != nullptr);
  assert(pContext->IsValid());
  assert(pTextureScreenShot != nullptr);
  assert(pTextureScreenShot->IsValid());  
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

  assert(pShaderScreenQuad != nullptr);
  assert(pShaderScreenQuad->IsCompiledProgram());
  assert(pStaticVertexBufferObjectScreenQuad != nullptr);
  assert(pStaticVertexBufferObjectScreenQuad->IsCompiled());

  const spitfire::math::cColour sunColour(0.2, 0.2, 0.0);

  // Setup materials
  const spitfire::math::cColour ambient(sunColour);
  pContext->SetMaterialAmbientColour(ambient);
  const spitfire::math::cColour diffuse(0.8, 0.1, 0.0);
  pContext->SetMaterialDiffuseColour(diffuse);
  const spitfire::math::cColour specular(1.0, 0.3, 0.3);
  pContext->SetMaterialSpecularColour(specular);
  const float fShininess = 50.0f;
  pContext->SetMaterialShininess(fShininess);


  // Setup lighting
  // NOTE: No lighting to show that the lightmap is working
  //pContext->EnableLighting();
  //pContext->EnableLight(0);
  //const spitfire::math::cVec3 lightPosition(5.0f, 5.0f, 10.0f);
  //pContext->SetLightPosition(0, lightPosition);
  //const spitfire::math::cColour lightAmbient(sunColour);
  //pContext->SetLightAmbientColour(0, lightAmbient);
  //const spitfire::math::cColour lightDiffuse(1.0, 1.0, 1.0);
  //pContext->SetLightDiffuseColour(0, lightDiffuse);
  //const spitfire::math::cColour lightSpecular(1.0f, 1.0f, 1.0f);
  //pContext->SetLightSpecularColour(0, lightSpecular);


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

  uint32_t previousTime = SDL_GetTicks();
  uint32_t currentTime = SDL_GetTicks();

  // Setup mouse
  pWindow->ShowCursor(false);
  pWindow->WarpCursorToMiddleOfScreen();


  uint32_t duration = (10 * 1000); // End in 10 seconds time

  uint32_t startTime = SDL_GetTicks();
  uint32_t endTime = startTime + duration;

  while (!bIsDone && (currentTime < endTime)) {
    // Update window events
    pWindow->UpdateEvents();

    // Update state
    previousTime = currentTime;
    currentTime = SDL_GetTicks();

    const float fBlend0 = (float(currentTime) - float(startTime)) / float(duration);
    const float fBlend1 = 1.0f - fBlend0;

    matRotation.SetRotation(rotationZ * rotationX);



    {
      // Render the scene into the frame buffer object
      const spitfire::math::cColour clearColour(0.0f, 1.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRenderToTexture(*pTextureFrameBufferObject);

      const spitfire::math::cVec3 offset = matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, -fZoom, 0.0f));
      const spitfire::math::cVec3 up = matRotation.GetRotatedVec3(spitfire::math::v3Up);

      const spitfire::math::cVec3 target(0.0f, 0.0f, 0.0f);
      const spitfire::math::cVec3 eye(target + offset);
      spitfire::math::cMat4 matModelView;
      matModelView.LookAt(eye, target, up);

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

      pContext->EndRenderToTexture(*pTextureFrameBufferObject);
    }

    {
      // Render the scene with the new texture
      const spitfire::math::cColour clearColour(1.0f, 0.0f, 0.0f);
      pContext->SetClearColour(clearColour);

      pContext->BeginRendering();

      // Now draw an overlay of our rendered texture
      pContext->BeginRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN);

      // Move the quad into the bottom right hand corner of the screen
      spitfire::math::cMat4 matModelView2D;
      matModelView2D.SetTranslation(0.5f, 0.5f, 0.0f);

      pContext->BindTexture(0, *pTextureScreenShot);
      pContext->BindTexture(1, *pTextureFrameBufferObject);

      pContext->BindShader(*pShaderScreenQuad);

      pContext->SetShaderConstant("fBlend0", fBlend0);
      pContext->SetShaderConstant("fBlend1", fBlend1);

      pContext->BindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenQuad);

      {
        pContext->SetModelViewMatrix(matModelView2D);

        pContext->DrawStaticVertexBufferObjectQuads2D(*pStaticVertexBufferObjectScreenQuad);
      }

      pContext->UnBindStaticVertexBufferObject2D(*pStaticVertexBufferObjectScreenQuad);

      pContext->UnBindShader(*pShaderScreenQuad);

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

  while (!bIsDone) {
    // Update window events
    pWindow->UpdateEvents();

    // Keep the cursor locked to the middle of the screen so that when the mouse moves, it is in relative pixels
    pWindow->WarpCursorToMiddleOfScreen();

    // Update state
    previousTime = currentTime;
    currentTime = SDL_GetTicks();

    matRotation.SetRotation(rotationZ * rotationX);


    // Render the scene
    const spitfire::math::cColour clearColour(0.392156863f, 0.584313725f, 0.929411765f);
    pContext->SetClearColour(clearColour);

    pContext->BeginRendering();

    if (bIsWireframe) pContext->EnableWireframe();

    const spitfire::math::cVec3 offset = matRotation.GetRotatedVec3(spitfire::math::cVec3(0.0f, -fZoom, 0.0f));
    const spitfire::math::cVec3 up = matRotation.GetRotatedVec3(spitfire::math::v3Up);

    const spitfire::math::cVec3 target(0.0f, 0.0f, 0.0f);
    const spitfire::math::cVec3 eye(target + offset);
    spitfire::math::cMat4 matModelView;
    matModelView.LookAt(eye, target, up);

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

    pContext->EndRendering();

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

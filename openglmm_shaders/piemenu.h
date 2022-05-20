#pragma once

#include <list>

// Spitfire headers
#include <spitfire/math/cMat4.h>

// libopenglmm headers
#include <libopenglmm/cFont.h>
#include <libopenglmm/cShader.h>
#include <libopenglmm/cTexture.h>
#include <libvoodoomm/cTextureAtlas.h>
#include <libopenglmm/cVertexBufferObject.h>

// Application headers
#include "util.h"

class cApplication;
class cGUI;

struct PieMenuItem {
  int idCommand;
  std::string caption;
  std::string iconFilePath;
};

class cPieMenu {
public:
  cPieMenu();

  void Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  bool IsOpen() const { return bIsOpen; }

  void SetMenuItems(opengl::cContext& context, opengl::cFont& font, const std::vector<PieMenuItem>& menuItems);

  void ShowAtPoint(int x, int y);
  void Hide() { bIsOpen = false; }

  bool OnMouseEvent(cGUI& gui, const opengl::cMouseEvent& event);

  void Render2D(opengl::cContext& context, opengl::cFont& font, int mouseX, int mouseY);

private:
  const PieMenuItem* GetMenuItemUnderPoint(int x, int y) const;

  void UpdateVertexBufferObjects(opengl::cContext& context, const voodoo::cTextureAtlas& textureAtlas);
  void CreateText(opengl::cContext& context, opengl::cFont& font);

  opengl::cTexture textureIcons;
  opengl::cShader shaderPassThroughColour;
  opengl::cShader shaderPassThroughTexture;
  opengl::cStaticVertexBufferObject circleVBO;
  opengl::cStaticVertexBufferObject menuItemVBO;
  opengl::cStaticVertexBufferObject menuItemIconsVBO;
  opengl::cStaticVertexBufferObject textVBO;

  bool bIsOpen;
  spitfire::math::cVec2 position;

  std::vector<PieMenuItem> menuItems;

  float fCircleRadius;
  float fMenuItemOuterRadius;
  float fMenuItemInnerRadius;
  size_t nCircleSegments;
  float fCircleSegmentArcDegrees;
  float fMenuItemSpaceArcDegrees;
  float fMenuItemInnerArcDegrees;
  float fMenuItemOuterArcDegrees;
  float fMenuItemSegmentInnerArcDegrees;
  float fMenuItemSegmentOuterArcDegrees;
  size_t nMenuItemSegments;
};

class cGUI {
public:
  explicit cGUI(cApplication& application);

  void Init(opengl::cContext& context);
  void Destroy(opengl::cContext& context);

  void SetPieMenuItems(opengl::cContext& context, const std::vector<PieMenuItem>& menuItems);

  bool IsMenuOpen() const { return pieMenu.IsOpen(); }

  void SendCommandToApplication(int idCommand);

  bool OnKeyUp(int keyCode, opengl::cContext& context);
  bool OnMouseEvent(const opengl::cMouseEvent& event);

  void Render2D(opengl::cContext& context);

private:
  cApplication& application;

  int mouseX;
  int mouseY;

  opengl::cFont fontRoboto;

  cPieMenu pieMenu;
};

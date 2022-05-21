#pragma once

// libopenglmm headers
#include <libopenglmm/cFont.h>

// Application headers
#include "piemenu.h"
#include "util.h"

class cApplication;

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

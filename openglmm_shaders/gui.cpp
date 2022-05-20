// Application headers
#include "main.h"
#include "gui.h"

cGUI::cGUI(cApplication& _application) :
  application(_application),
  mouseX(0),
  mouseY(0)
{
}

void cGUI::Init(opengl::cContext& context)
{
  // Create our font
  context.CreateFont(fontRoboto, TEXT("fonts/roboto-regular.ttf"), 50, TEXT("shaders/font.vert"), TEXT("shaders/font.frag"));
  assert(fontRoboto.IsValid());

  pieMenu.Init(context);
}

void cGUI::Destroy(opengl::cContext& context)
{
  pieMenu.Destroy(context);

  if (fontRoboto.IsValid()) context.DestroyFont(fontRoboto);
}

void cGUI::SetPieMenuItems(opengl::cContext& context, const std::vector<PieMenuItem>& menuItems)
{
  pieMenu.SetMenuItems(context, fontRoboto, menuItems);
}

void cGUI::SendCommandToApplication(int idCommand)
{
  application.OnCommand(idCommand);
}

bool cGUI::OnKeyUp(int keyCode, opengl::cContext& context)
{
  if (keyCode == SDLK_ESCAPE) {
    if (pieMenu.IsOpen()) {
      pieMenu.Hide();
      return true;
    }
  } else if (keyCode == SDLK_e) {
    pieMenu.ShowAtPoint(0.5f * context.GetWidth(), 0.5f * context.GetHeight());
    return true;
  }

  return false;
}

bool cGUI::OnMouseEvent(const opengl::cMouseEvent& event)
{
  // Keep track of the mouse location for all mouse events
  mouseX = event.GetX();
  mouseY = event.GetY();

  if (pieMenu.IsOpen()) {
    return pieMenu.OnMouseEvent(*this, event);
  }

  return false;
}

void cGUI::Render2D(opengl::cContext& context)
{
  if (pieMenu.IsOpen()) pieMenu.Render2D(context, fontRoboto, mouseX, mouseY);
}

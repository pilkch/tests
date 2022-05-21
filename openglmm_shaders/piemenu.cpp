// Spitfire headers
#include <spitfire/math/geometry.h>

// Application headers
#include "gui.h"
#include "main.h"
#include "piemenu.h"

// ** cPieMenu

cPieMenu::cPieMenu() :
  bIsOpen(false),
  fCircleRadius(0.0f),
  fMenuItemOuterRadius(0.0f),
  fMenuItemInnerRadius(0.0f),
  nCircleSegments(0),
  fCircleSegmentArcDegrees(0.0f),
  fMenuItemSpaceArcDegrees(0.0f),
  fMenuItemInnerArcDegrees(0.0f),
  fMenuItemOuterArcDegrees(0.0f),
  fMenuItemSegmentInnerArcDegrees(0.0f),
  fMenuItemSegmentOuterArcDegrees(0.0f),
  nMenuItemSegments(0)
{
}

void cPieMenu::Init(opengl::cContext& context)
{
  context.CreateShader(shaderPassThroughColour, TEXT("shaders/passthrough2dcolour.vert"), TEXT("shaders/passthrough2dcolour.frag"));
  assert(shaderPassThroughColour.IsCompiledProgram());
  context.CreateShader(shaderPassThroughTexture, TEXT("shaders/passthrough2d.vert"), TEXT("shaders/passthrough2dtexture.frag"));
  assert(shaderPassThroughTexture.IsCompiledProgram());
}

void cPieMenu::Destroy(opengl::cContext& context)
{
  context.DestroyShader(shaderPassThroughTexture);
  context.DestroyShader(shaderPassThroughColour);

  context.DestroyStaticVertexBufferObject(circleVBO);
  context.DestroyStaticVertexBufferObject(menuItemVBO);
  context.DestroyStaticVertexBufferObject(menuItemIconsVBO);
  context.DestroyStaticVertexBufferObject(textVBO);
}

void cPieMenu::SetMenuItems(opengl::cContext& context, opengl::cFont& font, const std::vector<PieMenuItem>& _menuItems)
{
  position.Set(0.5f * context.GetWidth(), 0.5f * context.GetHeight());

  menuItems = _menuItems;

  if (menuItems.size() > 1) {
    // Calculate the sizes of the menu items
    nCircleSegments = 0.1f * context.GetHeight();
    fCircleRadius = 0.15f * context.GetHeight();

    fCircleSegmentArcDegrees = 360.0f / float(nCircleSegments);

    const float fGap = 0.09f * fCircleRadius;
  
    fMenuItemInnerRadius = fCircleRadius + fGap;
    fMenuItemOuterRadius = fMenuItemInnerRadius + (1.0f * fCircleRadius);

    // NOTE: The menu items take up an equal portion of the loop
    const size_t nMenuItems = menuItems.size() - 1;
    const float fInnerCircumference = spitfire::math::CalculateCircleCircumference(fMenuItemInnerRadius);
    const float fOuterCircumference = spitfire::math::CalculateCircleCircumference(fMenuItemOuterRadius);

    // The area for the menu items
    const float fMenuItemInnerSpaceLengthOfCurve = fInnerCircumference / float(nMenuItems);
    const float fMenuItemOuterSpaceLengthOfCurve = fOuterCircumference / float(nMenuItems);

    // The actual drawn sizes of the menu items
    const float fMenuItemInnerLengthOfCurve = fMenuItemInnerSpaceLengthOfCurve - fGap;
    const float fMenuItemOuterLengthOfCurve = fMenuItemOuterSpaceLengthOfCurve - fGap;

    // There is a slight problem if we use the same arc angle for the gap between adjacent menu item inner and outer corner pairs, they will seem to converge or diverge.
    // So we use the gap length along the arc curve for the inner and outer, and from there we can get the arc angles we are after
    fMenuItemSpaceArcDegrees = 360.0f / float(nMenuItems); // The menu item width plus the space
    fMenuItemInnerArcDegrees = spitfire::math::CalculateArcAngleFromRadiusAndLengthOfArcCurve(fMenuItemInnerRadius, fMenuItemInnerLengthOfCurve);
    fMenuItemOuterArcDegrees = spitfire::math::CalculateArcAngleFromRadiusAndLengthOfArcCurve(fMenuItemOuterRadius, fMenuItemOuterLengthOfCurve);

    // Now calculate how many segments each menu item will be rendered with
    nMenuItemSegments = fMenuItemInnerArcDegrees / 5.0f; // Place a point every 5 degrees
    fMenuItemSegmentInnerArcDegrees = fMenuItemInnerArcDegrees / float(nMenuItemSegments);
    fMenuItemSegmentOuterArcDegrees = fMenuItemOuterArcDegrees / float(nMenuItemSegments);
  }

  voodoo::cTextureAtlas textureAtlas;

  {
    // We know the icons are all 26x26 and we have 9 of them (3x3), we then round to the next power of 2
    const size_t widthAndHeight = spitfire::math::NextPowerOfTwo(3 * 26);
    textureAtlas.Init(widthAndHeight, widthAndHeight, opengl::PIXELFORMAT::R8G8B8A8);

    for (size_t i = 0; i < menuItems.size(); i++) {
      assert(textureAtlas.AddImage(menuItems[i].iconFilePath));
    }

    // Create our texture from the image
    context.CreateTextureFromImageNoMipMaps(textureIcons, textureAtlas.GetImage());

    assert(textureIcons.GetWidth() == textureAtlas.GetWidth());
    assert(textureIcons.GetHeight() == textureAtlas.GetHeight());
    assert(textureIcons.GetPixelFormat() == textureAtlas.GetImage().GetPixelFormat());
  }

  UpdateVertexBufferObjects(context, textureAtlas);
  CreateText(context, font);
}

void cPieMenu::ShowAtPoint(int x, int y)
{
  bIsOpen = true;
  position.Set(float(x), float(y));
}

const PieMenuItem* cPieMenu::GetMenuItemUnderPoint(int x, int y) const
{
  if (menuItems.empty()) {
    return nullptr;
  }

  // Check if we are even within the radial menu at all
  const spitfire::math::cVec2 cursor(x, y);
  if (!IsPointInCircle(cursor, position, fMenuItemOuterRadius)) {
    return nullptr;
  }

  // Check if we are within the middle circle
  if (IsPointInCircle(cursor, position, fCircleRadius)) {
    return &menuItems[0];
  }

  // We must be within a menu item or empty space within the radial menu
  const float fStartingAngleDegrees = -0.5f * fMenuItemSpaceArcDegrees;
  for (size_t i = 1; i < menuItems.size(); i++) {
    const size_t direction = i - 1; // The first item is at the top center

    const float fAngleDegrees0 = fStartingAngleDegrees + (direction * (fMenuItemSpaceArcDegrees));
    const float fAngleDegrees1 = fStartingAngleDegrees + ((direction + 1) * (fMenuItemSpaceArcDegrees));

    // NOTE: We check a slightly larger triangle, we have already exited if we are in the general fMenuItemOuterRadius circle check above anyway
    const spitfire::math::cVec2 point0 = position + spitfire::math::CalculateCartesianCoordinate(fAngleDegrees0, 1.3f * fMenuItemOuterRadius);
    const spitfire::math::cVec2 point1 = position + spitfire::math::CalculateCartesianCoordinate(fAngleDegrees1, 1.3f * fMenuItemOuterRadius);

    if (IsPointInTriangle(cursor, position, point0, point1)) {
      return &menuItems[i];
    }
  }

  return nullptr;
}

void cPieMenu::UpdateVertexBufferObjects(opengl::cContext& context, const voodoo::cTextureAtlas& textureAtlas)
{
  if (circleVBO.IsCompiled()) circleVBO.Destroy();
  if (menuItemVBO.IsCompiled()) menuItemVBO.Destroy();
  if (menuItemIconsVBO.IsCompiled()) menuItemIconsVBO.Destroy();

  if (menuItems.empty()) return;

  // Create a circle
  {
    // TODO: Move this into breathe library
    opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

    opengl::cGeometryBuilder_v2 builder(*pGeometryDataPtr);

    const spitfire::math::cVec2 center;
    opengl::cGeometryBuilder::CreateCircle(builder, center, fCircleRadius, nCircleSegments);

    context.CreateStaticVertexBufferObject(circleVBO);

    circleVBO.SetData(pGeometryDataPtr);

    circleVBO.Compile2D();
  }


  // Create one menu item segment
  if (menuItems.size() > 1) {
    // TODO: Move this into breathe library
    opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

    opengl::cGeometryBuilder_v2 builder(*pGeometryDataPtr);

    // The first menu item is always centred at the top
    const float fStartingAngleDegrees = -0.5f * fMenuItemOuterArcDegrees;

    for (size_t i = 0; i < nMenuItemSegments; i++) {
      const float fInnerAngleDegrees0 = fStartingAngleDegrees + (i * fMenuItemSegmentInnerArcDegrees);
      const float fInnerAngleDegrees1 = fStartingAngleDegrees + ((i + 1) * fMenuItemSegmentInnerArcDegrees);
      const float fOuterAngleDegrees0 = fStartingAngleDegrees + (i * fMenuItemSegmentOuterArcDegrees);
      const float fOuterAngleDegrees1 = fStartingAngleDegrees + ((i + 1) * fMenuItemSegmentOuterArcDegrees);
      const spitfire::math::cVec2 point0 = spitfire::math::CalculateCartesianCoordinate(fInnerAngleDegrees0, fMenuItemInnerRadius);
      const spitfire::math::cVec2 point1 = spitfire::math::CalculateCartesianCoordinate(fOuterAngleDegrees0, fMenuItemOuterRadius);
      const spitfire::math::cVec2 point2 = spitfire::math::CalculateCartesianCoordinate(fOuterAngleDegrees1, fMenuItemOuterRadius);
      const spitfire::math::cVec2 point3 = spitfire::math::CalculateCartesianCoordinate(fInnerAngleDegrees1, fMenuItemInnerRadius);

      builder.PushBack(point0);
      builder.PushBack(point2);
      builder.PushBack(point1);
      builder.PushBack(point2);
      builder.PushBack(point0);
      builder.PushBack(point3);
    }

    context.CreateStaticVertexBufferObject(menuItemVBO);

    menuItemVBO.SetData(pGeometryDataPtr);

    menuItemVBO.Compile2D();
  }


  // Create our menu item icons
  if (menuItems.size() > 1) {
    // TODO: Move this into breathe library
    opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

    opengl::cGeometryBuilder_v2_t2 builder(*pGeometryDataPtr);

    // The first menu item is always centred at the top
    const float fStartingAngleDegrees = 0.0f;
    const float fTextRadius = 0.5f * (fMenuItemInnerRadius + fMenuItemOuterRadius); // Place the text exactly in the center of the menu item

    const spitfire::math::cVec2 centre(0.5f * context.GetWidth(), 0.5f * context.GetHeight());

    const size_t textureAtlasWidth = textureAtlas.GetWidth();
    const size_t textureAtlasHeight = textureAtlas.GetHeight();

    const size_t nMenuItems = menuItems.size();

    const std::vector<voodoo::cTextureAtlasEntry>& textureAtlasEntries = textureAtlas.GetTextureAtlasEntries();
    assert(textureAtlasEntries.size() == nMenuItems);

    // Icon on the center menu item
    {
      const spitfire::math::cVec2 textPosition = centre;

      const spitfire::math::cVec2 iconCenter = textPosition - spitfire::math::cVec2(0.0f, 0.04f * context.GetHeight());

      // NOTE: We know that these icons are actually 26x26 pixels
      const float fIconWidth = 26.0f * 2.0f;
      const float fIconHeight = fIconWidth * float(textureAtlasEntries[0].width) / float(textureAtlasEntries[0].height);
      const float fHalfIconWidth = 0.5f * fIconWidth;
      const float fHalfIconHeight = 0.5f * fIconHeight;
      const spitfire::math::cVec2 p0(iconCenter + spitfire::math::cVec2(-fHalfIconWidth, fHalfIconHeight));
      const spitfire::math::cVec2 p1(iconCenter + spitfire::math::cVec2(-fHalfIconWidth, -fHalfIconHeight));
      const spitfire::math::cVec2 p2(iconCenter + spitfire::math::cVec2(fHalfIconWidth, -fHalfIconHeight));
      const spitfire::math::cVec2 p3(iconCenter + spitfire::math::cVec2(fHalfIconWidth, fHalfIconHeight));

      // NOTE: We add a half pixel correction so that we are picking the centre of pixels and not the border between pixels
      // https://gamedev.stackexchange.com/a/49585/63400
      const spitfire::math::cVec2 uv0(float(textureAtlasEntries[0].x + 0.5f) / float(textureAtlasWidth), float(textureAtlasEntries[0].y - 0.5f + textureAtlasEntries[0].height) / float(textureAtlasHeight));
      const spitfire::math::cVec2 uv1(float(textureAtlasEntries[0].x - 0.5f + textureAtlasEntries[0].width) / float(textureAtlasWidth), float(textureAtlasEntries[0].y + 0.5f) / float(textureAtlasHeight));

      builder.PushBack(p0, spitfire::math::cVec2(uv0.x, uv0.y));
      builder.PushBack(p2, spitfire::math::cVec2(uv1.x, uv1.y));
      builder.PushBack(p1, spitfire::math::cVec2(uv0.x, uv1.y));
      builder.PushBack(p2, spitfire::math::cVec2(uv1.x, uv1.y));
      builder.PushBack(p0, spitfire::math::cVec2(uv0.x, uv0.y));
      builder.PushBack(p3, spitfire::math::cVec2(uv1.x, uv0.y));
    }

    // Icons on the surrounding menu items
    for (size_t i = 1; i < menuItems.size(); i++) {
      const float fInnerAngleDegrees = fStartingAngleDegrees + ((i - 1) * fMenuItemSpaceArcDegrees);
      const spitfire::math::cVec2 textPosition = centre + spitfire::math::CalculateCartesianCoordinate(fInnerAngleDegrees, fTextRadius) + spitfire::math::cVec2(0.0f, 0.02f * context.GetHeight());

      const spitfire::math::cVec2 iconCenter = textPosition - spitfire::math::cVec2(0.0f, 0.04f * context.GetHeight());

      // NOTE: We know that these icons are actually 26x26 pixels
      const float fIconWidth = 26.0f * 2.0f;
      const float fIconHeight = fIconWidth * float(textureAtlasEntries[i].width) / float(textureAtlasEntries[i].height);
      const float fHalfIconWidth = 0.5f * fIconWidth;
      const float fHalfIconHeight = 0.5f * fIconHeight;
      const spitfire::math::cVec2 p0(iconCenter + spitfire::math::cVec2(-fHalfIconWidth, fHalfIconHeight));
      const spitfire::math::cVec2 p1(iconCenter + spitfire::math::cVec2(-fHalfIconWidth, -fHalfIconHeight));
      const spitfire::math::cVec2 p2(iconCenter + spitfire::math::cVec2(fHalfIconWidth, -fHalfIconHeight));
      const spitfire::math::cVec2 p3(iconCenter + spitfire::math::cVec2(fHalfIconWidth, fHalfIconHeight));

      // NOTE: We add a half pixel correction so that we are picking the centre of pixels and not the border between pixels
      // https://gamedev.stackexchange.com/a/49585/63400
      const spitfire::math::cVec2 uv0(float(textureAtlasEntries[i].x + 0.5f) / float(textureAtlasWidth), float(textureAtlasEntries[i].y - 0.5f + textureAtlasEntries[i].height) / float(textureAtlasHeight));
      const spitfire::math::cVec2 uv1(float(textureAtlasEntries[i].x - 0.5f + textureAtlasEntries[i].width) / float(textureAtlasWidth), float(textureAtlasEntries[i].y + 0.5f) / float(textureAtlasHeight));

      builder.PushBack(p0, spitfire::math::cVec2(uv0.x, uv0.y));
      builder.PushBack(p2, spitfire::math::cVec2(uv1.x, uv1.y));
      builder.PushBack(p1, spitfire::math::cVec2(uv0.x, uv1.y));
      builder.PushBack(p2, spitfire::math::cVec2(uv1.x, uv1.y));
      builder.PushBack(p0, spitfire::math::cVec2(uv0.x, uv0.y));
      builder.PushBack(p3, spitfire::math::cVec2(uv1.x, uv0.y));
    }

    context.CreateStaticVertexBufferObject(menuItemIconsVBO);

    menuItemIconsVBO.SetData(pGeometryDataPtr);

    menuItemIconsVBO.Compile2D();
  }
}

void cPieMenu::CreateText(opengl::cContext& context, opengl::cFont& font)
{
  if (menuItems.empty()) return;

  // Destroy any existing VBO
  context.DestroyStaticVertexBufferObject(textVBO);

  opengl::cGeometryDataPtr pGeometryDataPtr = opengl::CreateGeometryData();

  opengl::cGeometryBuilder_v2_c4_t2 builder(*pGeometryDataPtr);

  const spitfire::math::cColour white(1.0f, 1.0f, 1.0f);

  const float fRotationDegrees = 0.0f;
  const spitfire::math::cVec2 scale(0.5f, 0.5f);

  //const float x = 0.7f * context.GetWidth();
  //float y = 0.5f * context.GetHeight();
  //font.PushBack(builder, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", white, opengl::cFont::FLAGS::HORIZONTAL_ALIGNMENT_LEFT_ALIGNED, spitfire::math::cVec2(x, y), fRotationDegrees, scale);
  //y += 30.0f;
  //font.PushBack(builder, "abcdefghijklmnopqrstuvwxyz", white, opengl::cFont::FLAGS::HORIZONTAL_ALIGNMENT_LEFT_ALIGNED, spitfire::math::cVec2(x, y), fRotationDegrees, scale);

  // Description at the top
  font.PushBack(builder, "Pie Menu", white, opengl::cFont::FLAGS::HORIZONTAL_ALIGNMENT_CENTERED, position - (spitfire::math::cVec2(0.0f, fMenuItemOuterRadius + 20.0f)), fRotationDegrees, scale);

  // The menu item in the center
  font.PushBack(builder, menuItems[0].caption, white, opengl::cFont::FLAGS::HORIZONTAL_ALIGNMENT_CENTERED, position, fRotationDegrees, scale);


  // The first menu item is always centred at the top
  const float fStartingAngleDegrees = 0.0f;
  const float fTextRadius = 0.5f * (fMenuItemInnerRadius + fMenuItemOuterRadius); // Place the text exactly in the center of the menu item

  for (size_t i = 1; i < menuItems.size(); i++) {
    const float fInnerAngleDegrees = fStartingAngleDegrees + ((i - 1) * fMenuItemSpaceArcDegrees);
    const spitfire::math::cVec2 point = spitfire::math::CalculateCartesianCoordinate(fInnerAngleDegrees, fTextRadius) + spitfire::math::cVec2(0.0f, 0.02f * context.GetHeight());

    font.PushBack(builder, menuItems[i].caption, white, opengl::cFont::FLAGS::HORIZONTAL_ALIGNMENT_CENTERED, position + point, fRotationDegrees, scale);
  }

  textVBO.SetData(pGeometryDataPtr);

  textVBO.Compile2D();
}

void cPieMenu::Render2D(opengl::cContext& context, opengl::cFont& font, int mouseX, int mouseY)
{
  if (!bIsOpen) return;

  if (menuItems.empty()) return;

  const PieMenuItem* pItem = GetMenuItemUnderPoint(mouseX, mouseY);

  const spitfire::math::cColour4 colourNormal(0.0f, 0.0f, 0.0f, 0.5f);
  //const spitfire::math::cColour4 colourHover(249.0f / 255.0f, 172.0f / 255.0f, 0.0f, 0.7f); // Orange
  const spitfire::math::cColour4 colourHover(51.0f / 255.0f, 186.0f / 255.0f, 0.0f, 0.7f); // Green

  context.BindShader(shaderPassThroughColour);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Draw the circle
  {
    context.SetShaderConstant("colour", (pItem == &menuItems[0]) ? colourHover : colourNormal);

    spitfire::math::cMat4 matModelView2D;
    matModelView2D.SetTranslation(position.x, position.y, 0.0f);

    context.BindStaticVertexBufferObject2D(circleVBO);
    context.SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_DIMENSIONS_AND_ASPECT_RATIO, matModelView2D);
    context.DrawStaticVertexBufferObjectTriangles2D(circleVBO);
    context.UnBindStaticVertexBufferObject2D(circleVBO);
  }


  // Draw the menu items
  for (size_t i = 1; i < menuItems.size(); i++) {
    const size_t direction = i - 1; // The first item is at the top center

    context.SetShaderConstant("colour", (pItem == &menuItems[i]) ? colourHover : colourNormal);

    spitfire::math::cMat4 matPosition;
    matPosition.SetTranslation(position.x, position.y, 0.0f);

    spitfire::math::cMat4 matRotation;
    matRotation.SetRotationZ(float(direction) * spitfire::math::DegreesToRadians(fMenuItemSpaceArcDegrees));

    context.BindStaticVertexBufferObject2D(menuItemVBO);
    context.SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_DIMENSIONS_AND_ASPECT_RATIO, matPosition * matRotation);
    context.DrawStaticVertexBufferObjectTriangles2D(menuItemVBO);
    context.UnBindStaticVertexBufferObject2D(menuItemVBO);
  }

  context.UnBindShader(shaderPassThroughColour);

  glDisable(GL_BLEND);

  // Draw the icons
  {
    context.BindShader(shaderPassThroughTexture);

    context.BindTexture(0, textureIcons);

    spitfire::math::cMat4 matPosition;

    context.BindStaticVertexBufferObject2D(menuItemIconsVBO);
    context.SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_DIMENSIONS_AND_ASPECT_RATIO, matPosition);
    context.DrawStaticVertexBufferObjectTriangles2D(menuItemIconsVBO);
    context.UnBindStaticVertexBufferObject2D(menuItemIconsVBO);

    context.UnBindTexture(0, textureIcons);

    context.UnBindShader(shaderPassThroughTexture);
  }

  // Draw the text overlays
  {
    context.BindFont(font);

    // Rendering the font in the middle of the screen
    spitfire::math::cMat4 matModelView;

    context.SetShaderProjectionAndModelViewMatricesRenderMode2D(opengl::MODE2D_TYPE::Y_INCREASES_DOWN_SCREEN_KEEP_DIMENSIONS_AND_ASPECT_RATIO, matModelView);

    context.BindStaticVertexBufferObject2D(textVBO);
    context.DrawStaticVertexBufferObjectTriangles2D(textVBO);
    context.UnBindStaticVertexBufferObject2D(textVBO);

    context.UnBindFont(font);
  }
}

bool cPieMenu::OnMouseEvent(cGUI& gui, const opengl::cMouseEvent& event)
{
  if (!bIsOpen) return false;

  // TODO: Make it so you have to click down and up on the menu item you want, not just whatever is under it when you release the button
  if (event.IsButtonUp() && (event.GetButton() == SDL_BUTTON_LEFT)) {
    // Left click
    const PieMenuItem* pItem = GetMenuItemUnderPoint(event.GetX(), event.GetY());
    if (pItem != nullptr) {
      // Tell the gui that an item was clicked
      gui.SendCommandToApplication(pItem->idCommand);

      // Dismiss the menu
      Hide();

      return true;
    } else {
      // Assume they clicked somewhere else, dismiss the menu
      Hide();
    }
  }

  return false;
}

class Anchor_t {
  foreign static TopLeft
  foreign static Top
  foreign static TopRight
  foreign static Left
  foreign static Center
  foreign static Right
  foreign static BottomLeft
  foreign static Bottom
  foreign static BottomRight
}

class CUI {
  foreign static getViewportWidth()
  foreign static getViewportHeight()

  foreign static setDrawColor(color)
  foreign static getDrawColor()
  foreign static setMaterial(material)

  foreign static setClipRect(x0, y0, x1, y1)
  foreign static resetClipRect()

  foreign static loadFont(name, atlasFilename, metricsFilename)
  foreign static setFont(font)
  foreign static setFontByName(name)
  foreign static setDefaultFont(font)
  foreign static getCurrentFont()

  foreign static setFontSize(size)
  foreign static getFontSize()

  foreign static setSoftness(softness)
  foreign static getSoftness()
  foreign static setLetterSpacing(spacing)
  foreign static getLetterSpacing()
  foreign static setLineSpacing(spacing)
  foreign static getLineSpacing()

  foreign static measureTextWidth(text)
  foreign static measureTextHeight(text)

  foreign static drawFilledRect(x0, y0, x1, y1)
  foreign static drawOutlinedRect(x0, y0, x1, y1)
  foreign static drawLine(x0, y0, x1, y1)

  foreign static drawText(text, x, y)
  foreign static drawTextOutline(text, x, y, outlineColor, outlineWidth)
  foreign static drawTextAnchored(text, anchor, x, y)
  foreign static drawTextAnchoredOutline(text, anchor, x, y, outlineColor, outlineWidth)

  foreign static beginDraw()
  foreign static endDraw()
}
class IRenderer {
  foreign static beginFrame()
  foreign static endFrame()
  foreign static clear(flags, color, depth, stencil)
  foreign static setFullViewport()
  foreign static getAspectRatio()
}
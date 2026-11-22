class TextureWrapMode_t {
  foreign static Repeat
  foreign static MirroredRepeat
  foreign static ClampToEdge
}

foreign class CTexture {
  foreign construct new()

  foreign loadFromFile(filename)
  foreign createFromMemory(width, height, pixelData)
  foreign createFromCubeMemory(size, faces)
  foreign createCubemapFromEquirect(filename, faceSize)

  foreign setWrapMode(mode)

  foreign getWrapMode

  foreign getHandle

  foreign getWidth

  foreign getHeight

  foreign getData
}
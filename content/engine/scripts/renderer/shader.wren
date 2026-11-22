class ShaderType_t {
  foreign static Default
  foreign static Skybox
}

foreign class CShader {
  foreign construct new()

  foreign loadFromFile(vertexPath, fragmentPath)
  foreign createFromSource(name)

  foreign setType(type)

  foreign getType

  foreign getHandle

  foreign getName

  foreign addTexture(texture, slot)
  foreign removeTexture(slot)
  foreign getTexture(slot)
}
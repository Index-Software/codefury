foreign class CMaterial {
  foreign construct new()

  foreign setShaderName(shaderName)
  foreign setShader(shader)
  foreign getShader
  foreign setTexture(texture)
  foreign getTexture
  foreign setRenderQueue(queue)
  foreign getRenderQueue
  foreign getAlphaBlending
  foreign setBlendEnabled(isEnabled)
  foreign setAdditive(isAdditive)
  foreign getHandle

  foreign setFloat(name, value)
  foreign setVector(name, value)
  foreign flushProperties()

  foreign getProperties
}
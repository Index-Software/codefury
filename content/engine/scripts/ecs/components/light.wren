class LightType_t {
  foreign static Point
  foreign static Directional
}

foreign class CLight {
  foreign construct new()

  foreign setColor(color)

  foreign getColor

  foreign setPosition(position)

  foreign getPosition

  foreign setDirection(direction)

  foreign getDirection

  foreign setIntensity(intensity)

  foreign getIntensity

  foreign setRadius(radius)

  foreign getRadius

  foreign setShadowCenterOffset(offset)

  foreign getShadowCenterOffset

  foreign setShadowDistance(distance)

  foreign getShadowDistance

  foreign setShadowOrthoSize(size)

  foreign getShadowOrthoSize

  foreign setShadowNearPlane(nearPlane)

  foreign getShadowNearPlane

  foreign setShadowFarPlane(farPlane)

  foreign getShadowFarPlane

  foreign setShadowResolution(resolution)

  foreign getShadowResolution

  foreign setShadowStrength(strength)

  foreign getShadowStrength

  foreign setShadowBias(bias)

  foreign getShadowBias

  foreign setType(type)

  foreign getType
}
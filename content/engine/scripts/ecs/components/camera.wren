foreign class CCamera {
  foreign construct new()

  foreign setPerspective(fov, aspect, nearPlane, farPlane)
  foreign setPosition(position)
  foreign lookAt(target, up)
  foreign setRotation(rotation)

  foreign getPosition

  foreign getTarget

  foreign getAspect

  foreign getFOV

  foreign getViewMatrix
  foreign getProjectionMatrix
  foreign getViewProjectionMatrix
}
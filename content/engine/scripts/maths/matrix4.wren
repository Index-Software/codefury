foreign class CMatrix4 {
  foreign construct new()

  foreign multiply(other)
  foreign add(other)
  foreign subtract(other)
  foreign equals(other)
  foreign notEquals(other)
  foreign toTransformMatrix(position, scale)
}
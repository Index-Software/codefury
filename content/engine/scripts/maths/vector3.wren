foreign class CVector3 {
  foreign construct new(x, y, z)

  foreign x
  foreign y
  foreign z

  foreign length
  foreign lengthSq
  foreign lengthXY
  foreign distance(other)
  foreign distanceSq(other)
  foreign dot(other)
  foreign cross(other)

  foreign add(other)
  foreign subtract(other)
  foreign multiply(other)
  foreign divide(other)
  foreign multiplyScalar(s)
  foreign divideScalar(s)
  foreign equals(other)
  foreign notEquals(other)
}
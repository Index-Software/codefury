class CMaths {
  foreign static Pi
  foreign static TwoPi
  foreign static Deg2Rad
  foreign static Rad2Deg

  foreign static sqrt(x)
  foreign static tan(x)
  foreign static cos(x)
  foreign static sin(x)
  foreign static atan2(y, x)
  foreign static asin(x)
  foreign static abs(x)
  foreign static min(a, b)
  foreign static max(a, b)
  foreign static clamp(value, minVal, maxVal)

  foreign static normalise(v)
  foreign static cross(a, b)
  foreign static dot(a, b)

  foreign static perspective(fovyDeg, aspect, nearClip, farClip)
  foreign static orthographic(left, right, bottom, top, nearClip, farClip)
  foreign static lookAt(eye, target, up)
  foreign static translate(m, v)
  foreign static rotate(m, angleDeg, axis)
  foreign static scale(m, v)

  foreign static directionToEquirect(dir)
}
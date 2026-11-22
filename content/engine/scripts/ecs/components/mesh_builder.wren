class FaceOrientation_t {
  foreign static Inwards
  foreign static Outwards
}

class CMeshBuilder {
  foreign static createCube(material, halfSize, color, faceOrientation, subdivisions)

  foreign static createSphere(material, radius, rings, segments, color, faceOrientation)

  foreign static createPlane(material, halfSize, color, faceOrientation, subdivisions)
}

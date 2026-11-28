class IInput {
  foreign static setMouseCaptured(isCaptured)
  foreign static getMouseCaptured()

  foreign static getLookX()
  foreign static getLookY()
  foreign static getMoveX()
  foreign static getMoveY()

  foreign static isKeyDown(key)
  foreign static wasKeyPressed(key)
  foreign static wasKeyReleased(key)
  foreign static getKeyEventCount()
  foreign static getKeyEventKey(index)
  foreign static getKeyEventMods(index)
  foreign static wasKeyEventPressed(index)
  foreign static wasKeyEventReleased(index)
  foreign static getKeyEventTime(index)
}
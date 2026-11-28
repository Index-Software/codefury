class CInputActions {
  foreign static bind(name, key, mods)
  foreign static setBinding(name, index, key, mods)
  foreign static clearBindings(name)
  foreign static getBindingCount(name)
  foreign static getBindingKey(name, index)
  foreign static getBindingMods(name, index)

  foreign static isDown(name)
  foreign static wasPressed(name)
  foreign static wasReleased(name)
}

class InputMods_t {
  foreign static None
  foreign static Shift
  foreign static Control
  foreign static Alt
  foreign static Super
}
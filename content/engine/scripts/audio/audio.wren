class IAudio {
  foreign static loadSound(path)
  foreign static unloadSound(sound)
  foreign static playSound(sound)
  foreign static stopSound(instance)
  foreign static isSoundPlaying(instance)
  foreign static getSoundTime(instance)
}
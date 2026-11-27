class IAudio {
  foreign static loadSound(path)
  foreign static unloadSound(sound)
  foreign static playSound(sound)
  foreign static playSoundAt(sound, seconds)
  foreign static stopSound(instance)
  foreign static isSoundPlaying(instance)
  foreign static getSoundTime(instance)
  foreign static setSoundPaused(instance, isPaused)
  foreign static setSoundVolume(instance, volume)
}
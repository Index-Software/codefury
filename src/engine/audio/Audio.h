//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef AUDIO_H
#define AUDIO_H

#pragma once

#include "platform.h"
#include "WrenBind.h"

typedef uint32 SoundHandle;
typedef uint32 SoundInstanceHandle;

static const SoundHandle s_InvalidSoundHandle = 0;
static const SoundInstanceHandle s_InvalidSoundInstanceHandle = 0;

class IAudio : public CWrenBindSingleton<IAudio>
{
	WREN_SINGLETON_DECL()
public:
	virtual ~IAudio()
	{
	}

	virtual int32 Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Update() = 0;

	virtual SoundHandle LoadSound(const char* pPath)
		WREN_BIND_SINGLETON("loadSound(_)", s_pAudio) = 0;
	virtual void UnloadSound(SoundHandle hSound)
		WREN_BIND_SINGLETON("unloadSound(_)", s_pAudio) = 0;
	virtual SoundInstanceHandle PlaySound(SoundHandle hSound)
		WREN_BIND_SINGLETON("playSound(_)", s_pAudio) = 0;
	virtual SoundInstanceHandle PlaySoundAt(SoundHandle hSound, float64 seconds)
		WREN_BIND_SINGLETON("playSoundAt(_,_)", s_pAudio) = 0;
	virtual void StopSound(SoundInstanceHandle hInstance)
		WREN_BIND_SINGLETON("stopSound(_)", s_pAudio) = 0;
	virtual bool IsSoundPlaying(SoundInstanceHandle hInstance)
		WREN_BIND_SINGLETON("isSoundPlaying(_)", s_pAudio) = 0;
	virtual float64 GetSoundTime(SoundInstanceHandle hInstance)
		WREN_BIND_SINGLETON("getSoundTime(_)", s_pAudio) = 0;
	virtual void SetSoundPaused(SoundInstanceHandle hInstance, bool isPaused)
		WREN_BIND_SINGLETON("setSoundPaused(_,_)", s_pAudio) = 0;
	virtual void SetSoundVolume(SoundInstanceHandle hInstance, float32 volume)
		WREN_BIND_SINGLETON("setSoundVolume(_,_)", s_pAudio) = 0;
};

IAudio* CreateAudio();

#endif // AUDIO_H
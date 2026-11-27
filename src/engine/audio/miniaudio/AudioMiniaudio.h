//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef AUDIO_MINIAUDIO_H
#define AUDIO_MINIAUDIO_H

#pragma once

#include "../Audio.h"
#include "../AudioMixer.h"
#include <miniaudio/miniaudio.h>

class CAudioMiniaudio : public IAudio
{
public:
	CAudioMiniaudio();
	~CAudioMiniaudio() CF_OVERRIDE;

	int32 Init() CF_OVERRIDE;
	void Shutdown() CF_OVERRIDE;
	void Update() CF_OVERRIDE;

	SoundHandle LoadSound(const char* pPath) CF_OVERRIDE;
	void UnloadSound(SoundHandle hSound) CF_OVERRIDE;
	SoundInstanceHandle PlaySound(SoundHandle hSound) CF_OVERRIDE;
	SoundInstanceHandle PlaySoundAt(
		SoundHandle hSound,
		float64 seconds) CF_OVERRIDE;
	void StopSound(SoundInstanceHandle hInstance) CF_OVERRIDE;
	bool IsSoundPlaying(SoundInstanceHandle hInstance) CF_OVERRIDE;
	float64 GetSoundTime(SoundInstanceHandle hInstance) CF_OVERRIDE;
	void SetSoundPaused(
		SoundInstanceHandle hInstance,
		bool isPaused) CF_OVERRIDE;
	void SetSoundVolume(
		SoundInstanceHandle hInstance,
		float32 volume) CF_OVERRIDE;
private:
	static void DataCallback(
		ma_device* pDevice,
		void* pOutput,
		const void* pInput,
		ma_uint32 frameCount);

	CAudioMixer m_Mixer;
	ma_device m_Device;
	bool m_IsDeviceInitialized;
};

#endif // AUDIO_MINIAUDIO_H
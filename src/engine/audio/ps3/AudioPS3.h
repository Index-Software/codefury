//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef AUDIO_PS3_H
#define AUDIO_PS3_H

#pragma once

#include "../Audio.h"
#include "../AudioMixer.h"
#include <audio/audio.h>
#include <sys/event_queue.h>
#include <sys/thread.h>

class CAudioPS3 : public IAudio
{
public:
	CAudioPS3();
	~CAudioPS3() CF_OVERRIDE;

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
	static void AudioThread(void* pArg);

	CAudioMixer m_Mixer;
	audioPortConfig m_PortConfig;
	sys_ipc_key_t m_QueueKey;
	sys_ppu_thread_t m_Thread;
	sys_event_queue_t m_EventQueue;
	uint32 m_Port;
	volatile bool m_IsRunning;
	bool m_IsAudioInitialized;
	bool m_IsPortOpen;
	bool m_IsQueueCreated;
	bool m_IsThreadCreated;
};

#endif // AUDIO_PS3_H
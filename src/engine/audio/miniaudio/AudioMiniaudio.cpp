//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "AudioMiniaudio.h"
#include "utils/UtlMemory.h"
#include "dbg.h"
#include <new>

CAudioMiniaudio::CAudioMiniaudio() :
	m_IsDeviceInitialized(false)
{
}

CAudioMiniaudio::~CAudioMiniaudio()
{
	Shutdown();
}

int32 CAudioMiniaudio::Init()
{
	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_f32;
	config.playback.channels = s_AudioOutputChannelCount;
	config.sampleRate = s_AudioSampleRate;
	config.dataCallback = DataCallback;
	config.pUserData = this;

	if (ma_device_init(CF_NULL, &config, &m_Device) != MA_SUCCESS)
	{
		Error("[CAudioMiniaudio] Failed to initialize device\n");

		return 0;
	}

	m_IsDeviceInitialized = true;

	if (ma_device_start(&m_Device) != MA_SUCCESS)
	{
		Error("[CAudioMiniaudio] Failed to start device\n");

		Shutdown();

		return 0;
	}

	return 1;
}

void CAudioMiniaudio::Shutdown()
{
	if (m_IsDeviceInitialized)
	{
		ma_device_uninit(&m_Device);
		m_IsDeviceInitialized = false;
	}

	m_Mixer.Shutdown();
}

void CAudioMiniaudio::Update()
{
	m_Mixer.Update();
}

SoundHandle CAudioMiniaudio::LoadSound(const char* pPath)
{
	return m_Mixer.LoadSound(pPath);
}

void CAudioMiniaudio::UnloadSound(SoundHandle hSound)
{
	m_Mixer.UnloadSound(hSound);
}

SoundInstanceHandle CAudioMiniaudio::PlaySound(SoundHandle hSound)
{
	return m_Mixer.PlaySound(hSound);
}

void CAudioMiniaudio::StopSound(SoundInstanceHandle hInstance)
{
	m_Mixer.StopSound(hInstance);
}

void CAudioMiniaudio::DataCallback(
	ma_device* pDevice,
	void* pOutput,
	const void* pInput,
	ma_uint32 frameCount)
{
	CAudioMiniaudio* pAudio =
		static_cast<CAudioMiniaudio*>(pDevice->pUserData);
	pAudio->m_Mixer.Mix(static_cast<float32*>(pOutput), frameCount);
}

IAudio* CreateAudio()
{
	void* pMemory = CUtlMemory::Alloc(sizeof(CAudioMiniaudio));
	if (pMemory)
	{
		return new(pMemory) CAudioMiniaudio();
	}

	return CF_NULL;
}
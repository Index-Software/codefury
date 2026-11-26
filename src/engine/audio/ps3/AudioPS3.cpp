//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "AudioPS3.h"
#include "utils/UtlMemory.h"
#include "dbg.h"
#include <string.h>
#include <new>

CAudioPS3::CAudioPS3() :
	m_QueueKey(0),
	m_Thread(0),
	m_EventQueue(0),
	m_Port(0),
	m_IsRunning(false),
	m_IsAudioInitialized(false),
	m_IsPortOpen(false),
	m_IsQueueCreated(false),
	m_IsThreadCreated(false)
{
	memset(&m_PortConfig, 0, sizeof(m_PortConfig));
}

CAudioPS3::~CAudioPS3()
{
	Shutdown();
}

int32 CAudioPS3::Init()
{
	if (audioInit() != 0)
	{
		Error("[CAudioPS3] Failed to initialize audio\n");

		return 0;
	}

	m_IsAudioInitialized = true;

	audioPortParam params;
	params.numChannels = AUDIO_PORT_2CH;
	params.numBlocks = AUDIO_BLOCK_8;
	params.attrib = 0;
	params.level = 1.0f;
	if (audioPortOpen(&params, &m_Port) != 0)
	{
		Error("[CAudioPS3] Failed to open audio port\n");

		Shutdown();

		return 0;
	}

	m_IsPortOpen = true;

	if (audioGetPortConfig(m_Port, &m_PortConfig) != 0)
	{
		Error("[CAudioPS3] Failed to get audio port config\n");

		Shutdown();

		return 0;
	}

	if (audioCreateNotifyEventQueue(&m_EventQueue, &m_QueueKey) != 0)
	{
		Error("[CAudioPS3] Failed to create notify event queue\n");

		Shutdown();

		return 0;
	}

	m_IsQueueCreated = true;

	if (audioSetNotifyEventQueue(m_QueueKey) != 0)
	{
		Error("[CAudioPS3] Failed to set notify event queue\n");

		Shutdown();

		return 0;
	}

	sysEventQueueDrain(m_EventQueue);

	m_IsRunning = true;
	const int32 threadCreateResult = sysThreadCreate(
		&m_Thread,
		AudioThread,
		this,
		100,
		16 * 1024,
		THREAD_JOINABLE,
		const_cast<char*>("AudioThread"));
	if (threadCreateResult != 0)
	{
		Error(
			"[CAudioPS3] Failed to create audio thread: %d\n",
			threadCreateResult);

		m_IsRunning = false;
		Shutdown();

		return 0;
	}

	m_IsThreadCreated = true;

	if (audioPortStart(m_Port) != 0)
	{
		Error("[CAudioPS3] Failed to start audio port\n");

		Shutdown();

		return 0;
	}

	return 1;
}

void CAudioPS3::Shutdown()
{
	if (m_IsThreadCreated)
	{
		m_IsRunning = false;

		uint64 exitCode;
		sysThreadJoin(m_Thread, reinterpret_cast<u64*>(&exitCode));
		m_IsThreadCreated = false;
	}

	if (m_IsPortOpen)
	{
		audioPortStop(m_Port);
	}

	if (m_IsQueueCreated)
	{
		audioRemoveNotifyEventQueue(m_QueueKey);
	}

	if (m_IsPortOpen)
	{
		audioPortClose(m_Port);
		m_IsPortOpen = false;
	}

	if (m_IsQueueCreated)
	{
		sysEventQueueDestroy(m_EventQueue, 0);
		m_IsQueueCreated = false;
	}

	if (m_IsAudioInitialized)
	{
		audioQuit();
		m_IsAudioInitialized = false;
	}

	m_Mixer.Shutdown();
}

void CAudioPS3::Update()
{
	m_Mixer.Update();
}

SoundHandle CAudioPS3::LoadSound(const char* pPath)
{
	return m_Mixer.LoadSound(pPath);
}

void CAudioPS3::UnloadSound(SoundHandle hSound)
{
	m_Mixer.UnloadSound(hSound);
}

SoundInstanceHandle CAudioPS3::PlaySound(SoundHandle hSound)
{
	return m_Mixer.PlaySound(hSound);
}

void CAudioPS3::StopSound(SoundInstanceHandle hInstance)
{
	m_Mixer.StopSound(hInstance);
}

void CAudioPS3::AudioThread(void* pArg)
{
	CAudioPS3* pAudio = static_cast<CAudioPS3*>(pArg);
	const audioPortConfig& portConfig = pAudio->m_PortConfig;
	volatile uint64* pReadIndex =
		reinterpret_cast<volatile uint64*>(uintp(portConfig.readIndex));
	float32* pDataStart =
		reinterpret_cast<float32*>(uintp(portConfig.audioDataStart));
	const uint32 blockSampleCount =
		uint32(portConfig.channelCount) * AUDIO_BLOCK_SAMPLES;

	while (pAudio->m_IsRunning)
	{
		sys_event_t event;
		if (sysEventQueueReceive(pAudio->m_EventQueue, &event, 20 * 1000) != 0)
		{
			continue;
		}

		// Fill the block after the one the hardware is reading
		const uint32 blockIndex =
			uint32((*pReadIndex + 1) % portConfig.numBlocks);
		pAudio->m_Mixer.Mix(
			pDataStart + blockIndex * blockSampleCount,
			AUDIO_BLOCK_SAMPLES);
	}

	sysThreadExit(0);
}

IAudio* CreateAudio()
{
	void* pMemory = CUtlMemory::Alloc(sizeof(CAudioPS3));
	if (pMemory)
	{
		return new(pMemory) CAudioPS3();
	}

	return CF_NULL;
}
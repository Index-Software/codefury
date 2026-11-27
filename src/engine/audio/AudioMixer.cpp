//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "audio/AudioMixer.h"
#include "audio/Vorbis.h"
#include "utils/UtlMemory.h"
#include "utils/UtlVector.h"
#include "utils/Utils.h"
#include "dbg.h"
#include <string.h>

STATIC_ASSERT(
	s_AudioEventCount >= s_AudioMaxChannels + s_AudioMaxSounds);

// Generation in the high 16 bits, index + 1 in the low 16 so 0 stays invalid
static uint32 MakeHandle(uint32 index, uint16 generation)
{
	return (uint32(generation) << 16) | (index + 1);
}

static uint32 GetHandleIndex(uint32 handle)
{
	return (handle & 0xFFFF) - 1;
}

static uint16 GetHandleGeneration(uint32 handle)
{
	return uint16(handle >> 16);
}

CAudioMixer::CAudioMixer()
{
	memset(m_MixerChannels, 0, sizeof(m_MixerChannels));
	memset(m_Channels, 0, sizeof(m_Channels));
	for (uint32 i = 0; i < s_AudioMaxChannels; ++i)
	{
		m_ChannelFrames[i] = 0;
	}
}

CAudioMixer::~CAudioMixer()
{
	Shutdown();
}

void CAudioMixer::Shutdown()
{
	for (uint32 i = 0; i < s_AudioMaxSounds; ++i)
	{
		SoundSlot_t& slot = m_Sounds[i];
		if (slot.m_State != SoundState_t::Free)
		{
			CUtlMemory::Free(slot.m_Sound.m_pSamples);
			slot.m_Sound = Sound_t();
			slot.m_State = SoundState_t::Free;
		}
	}

	memset(m_MixerChannels, 0, sizeof(m_MixerChannels));
	memset(m_Channels, 0, sizeof(m_Channels));
}

SoundHandle CAudioMixer::LoadSound(const char* pPath)
{
	uint32 slotIndex = s_AudioMaxSounds;
	for (uint32 i = 0; i < s_AudioMaxSounds; ++i)
	{
		if (m_Sounds[i].m_State == SoundState_t::Free)
		{
			slotIndex = i;

			break;
		}
	}

	if (slotIndex == s_AudioMaxSounds)
	{
		Error("[CAudioMixer] No free sound slot for %s\n", pPath);

		return s_InvalidSoundHandle;
	}

	CUtlVector<uint8> data = CUtils::ReadBinaryFile(CFixedString(pPath));
	if (data.IsEmpty())
	{
		Error("[CAudioMixer] Failed to read %s\n", pPath);

		return s_InvalidSoundHandle;
	}

	SoundSlot_t& slot = m_Sounds[slotIndex];
	if (!DecodeVorbis(data.Base(), uint32(data.Count()), slot.m_Sound))
	{
		Error("[CAudioMixer] Failed to decode %s\n", pPath);

		return s_InvalidSoundHandle;
	}

	slot.m_State = SoundState_t::Loaded;

	return MakeHandle(slotIndex, slot.m_Generation);
}

void CAudioMixer::UnloadSound(SoundHandle hSound)
{
	SoundSlot_t* pSlot = GetSoundSlot(hSound);
	if (!pSlot)
	{
		return;
	}

	AudioCommand_t command;
	memset(&command, 0, sizeof(command));
	command.m_Type = AudioCommandType_t::StopSound;
	command.m_SoundIndex = GetHandleIndex(hSound);
	if (!m_Commands.Push(command))
	{
		Warning("[CAudioMixer] Command queue full, sound not unloaded\n");

		return;
	}

	pSlot->m_State = SoundState_t::Unloading;
}

SoundInstanceHandle CAudioMixer::PlaySound(SoundHandle hSound)
{
	SoundSlot_t* pSlot = GetSoundSlot(hSound);
	if (!pSlot)
	{
		return s_InvalidSoundInstanceHandle;
	}

	uint32 channelIndex = s_AudioMaxChannels;
	for (uint32 i = 0; i < s_AudioMaxChannels; ++i)
	{
		if (!m_Channels[i].m_IsActive)
		{
			channelIndex = i;

			break;
		}
	}

	if (channelIndex == s_AudioMaxChannels)
	{
		Warning("[CAudioMixer] No free channel\n");

		return s_InvalidSoundInstanceHandle;
	}

	const Sound_t& sound = pSlot->m_Sound;

	AudioCommand_t command;
	command.m_pSamples = sound.m_pSamples;
	// 32.32 fixed point source frames per output frame
	command.m_Step = (uint64(sound.m_SampleRate) << 32) / s_AudioSampleRate;
	command.m_FrameCount = sound.m_FrameCount;
	command.m_SoundIndex = GetHandleIndex(hSound);
	command.m_ChannelIndex = channelIndex;
	command.m_ChannelCount = sound.m_ChannelCount;
	command.m_Type = AudioCommandType_t::Play;
	if (!m_Commands.Push(command))
	{
		Warning("[CAudioMixer] Command queue full, sound not played\n");

		return s_InvalidSoundInstanceHandle;
	}

	ChannelSlot_t& channel = m_Channels[channelIndex];
	channel.m_SampleRate = sound.m_SampleRate;
	channel.m_Generation++;
	channel.m_IsActive = true;
	m_ChannelFrames[channelIndex] = 0;

	return MakeHandle(channelIndex, channel.m_Generation);
}

void CAudioMixer::StopSound(SoundInstanceHandle hInstance)
{
	if (!GetChannelSlot(hInstance))
	{
		return;
	}

	AudioCommand_t command;
	memset(&command, 0, sizeof(command));
	command.m_Type = AudioCommandType_t::Stop;
	command.m_ChannelIndex = GetHandleIndex(hInstance);
	if (!m_Commands.Push(command))
	{
		Warning("[CAudioMixer] Command queue full, sound not stopped\n");
	}
}

bool CAudioMixer::IsSoundPlaying(SoundInstanceHandle hInstance) const
{
	return GetChannelSlot(hInstance) != CF_NULL;
}

float64 CAudioMixer::GetSoundTime(SoundInstanceHandle hInstance) const
{
	const ChannelSlot_t* pSlot = GetChannelSlot(hInstance);
	if (!pSlot)
	{
		return 0.0;
	}

	const uint32 frame = m_ChannelFrames[GetHandleIndex(hInstance)];

	return float64(frame) / float64(pSlot->m_SampleRate);
}

void CAudioMixer::Update()
{
	AudioEvent_t event;
	while (m_Events.Pop(event))
	{
		if (event.m_Type == AudioEventType_t::ChannelFinished)
		{
			m_Channels[event.m_Index].m_IsActive = false;
		}
		else
		{
			SoundSlot_t& slot = m_Sounds[event.m_Index];
			CUtlMemory::Free(slot.m_Sound.m_pSamples);
			slot.m_Sound = Sound_t();
			slot.m_Generation++;
			slot.m_State = SoundState_t::Free;
		}
	}
}

void CAudioMixer::Mix(float32* pOutput, uint32 frameCount)
{
	ProcessCommands();

	const uint32 sampleCount = frameCount * s_AudioOutputChannelCount;
	memset(pOutput, 0, sampleCount * sizeof(float32));

	for (uint32 i = 0; i < s_AudioMaxChannels; ++i)
	{
		MixerChannel_t& channel = m_MixerChannels[i];
		if (!channel.m_IsPlaying)
		{
			continue;
		}

		const bool isPlaying = MixChannel(channel, pOutput, frameCount);
		m_ChannelFrames[i] = uint32(channel.m_Position >> 32);
		if (!isPlaying)
		{
			StopChannel(i);
		}
	}

	for (uint32 i = 0; i < sampleCount; ++i)
	{
		const float32 sample = pOutput[i];
		pOutput[i] = sample > 1.0f ? 1.0f : (sample < -1.0f ? -1.0f : sample);
	}
}

SoundSlot_t* CAudioMixer::GetSoundSlot(SoundHandle hSound)
{
	const uint32 soundIndex = GetHandleIndex(hSound);
	if (soundIndex >= s_AudioMaxSounds)
	{
		return CF_NULL;
	}

	SoundSlot_t& slot = m_Sounds[soundIndex];
	if (slot.m_State != SoundState_t::Loaded
		|| slot.m_Generation != GetHandleGeneration(hSound))
	{
		return CF_NULL;
	}

	return &slot;
}

const ChannelSlot_t* CAudioMixer::GetChannelSlot(
	SoundInstanceHandle hInstance) const
{
	const uint32 channelIndex = GetHandleIndex(hInstance);
	if (channelIndex >= s_AudioMaxChannels)
	{
		return CF_NULL;
	}

	const ChannelSlot_t& slot = m_Channels[channelIndex];
	if (!slot.m_IsActive
		|| slot.m_Generation != GetHandleGeneration(hInstance))
	{
		return CF_NULL;
	}

	return &slot;
}

void CAudioMixer::ProcessCommands()
{
	AudioCommand_t command;
	while (m_Commands.Pop(command))
	{
		if (command.m_Type == AudioCommandType_t::Play)
		{
			MixerChannel_t& channel = m_MixerChannels[command.m_ChannelIndex];
			channel.m_pSamples = command.m_pSamples;
			channel.m_Position = 0;
			channel.m_Step = command.m_Step;
			channel.m_FrameCount = command.m_FrameCount;
			channel.m_SoundIndex = command.m_SoundIndex;
			channel.m_ChannelCount = command.m_ChannelCount;
			channel.m_IsPlaying = true;
		}
		else if (command.m_Type == AudioCommandType_t::Stop)
		{
			if (m_MixerChannels[command.m_ChannelIndex].m_IsPlaying)
			{
				StopChannel(command.m_ChannelIndex);
			}
		}
		else
		{
			for (uint32 i = 0; i < s_AudioMaxChannels; ++i)
			{
				MixerChannel_t& channel = m_MixerChannels[i];
				if (channel.m_IsPlaying
					&& channel.m_SoundIndex == command.m_SoundIndex)
				{
					StopChannel(i);
				}
			}

			AudioEvent_t event;
			event.m_Index = command.m_SoundIndex;
			event.m_Type = AudioEventType_t::SoundReleased;
			m_Events.Push(event);
		}
	}
}

bool CAudioMixer::MixChannel(
	MixerChannel_t& channel,
	float32* pOutput,
	uint32 frameCount)
{
	const float32 sampleScale = 1.0f / 32768.0f;
	const float32 fractionScale = 1.0f / 4294967296.0f;
	const uint64 endPosition = uint64(channel.m_FrameCount) << 32;
	const uint32 lastFrame = channel.m_FrameCount - 1;
	const int16* pSamples = channel.m_pSamples;
	uint64 position = channel.m_Position;

	for (uint32 i = 0; i < frameCount; ++i)
	{
		if (position >= endPosition)
		{
			channel.m_Position = position;

			return false;
		}

		const uint32 frame = uint32(position >> 32);
		const uint32 nextFrame = frame < lastFrame ? frame + 1 : frame;
		const float32 fraction =
			float32(uint32(position & 0xFFFFFFFF)) * fractionScale;

		float32 left;
		float32 right;
		if (channel.m_ChannelCount == 1)
		{
			const float32 a = float32(pSamples[frame]);
			const float32 b = float32(pSamples[nextFrame]);
			left = (a + (b - a) * fraction) * sampleScale;
			right = left;
		}
		else
		{
			const float32 aLeft = float32(pSamples[frame * 2]);
			const float32 aRight = float32(pSamples[frame * 2 + 1]);
			const float32 bLeft = float32(pSamples[nextFrame * 2]);
			const float32 bRight = float32(pSamples[nextFrame * 2 + 1]);
			left = (aLeft + (bLeft - aLeft) * fraction) * sampleScale;
			right = (aRight + (bRight - aRight) * fraction) * sampleScale;
		}

		pOutput[i * 2] += left;
		pOutput[i * 2 + 1] += right;
		position += channel.m_Step;
	}

	channel.m_Position = position;

	return true;
}

void CAudioMixer::StopChannel(uint32 channelIndex)
{
	m_MixerChannels[channelIndex].m_IsPlaying = false;

	AudioEvent_t event;
	event.m_Index = channelIndex;
	event.m_Type = AudioEventType_t::ChannelFinished;
	m_Events.Push(event);
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#pragma once

#include "audio/Audio.h"
#include "audio/Sound.h"

static const uint32 s_AudioSampleRate = 48000;
static const uint32 s_AudioOutputChannelCount = 2;
static const uint32 s_AudioMaxSounds = 128;
static const uint32 s_AudioMaxChannels = 32;
static const uint32 s_AudioCommandCount = 256;
static const uint32 s_AudioEventCount = 256;
static const uint32 s_AudioDecodeChunk = 4096;
static const uint32 s_AudioDecodeLead = 2;

struct stb_vorbis;

template<typename T, uint32 Size>
class CAudioQueue
{
public:
	CAudioQueue() :
		m_Read(0),
		m_Write(0)
	{
	}

	bool Push(const T& item)
	{
		const uint32 write = m_Write;
		if (write - m_Read == Size)
		{
			return false;
		}

		m_Items[write & (Size - 1)] = item;
		__sync_synchronize();
		m_Write = write + 1;

		return true;
	}

	bool Pop(T& outItem)
	{
		const uint32 read = m_Read;
		if (read == m_Write)
		{
			return false;
		}

		__sync_synchronize();
		outItem = m_Items[read & (Size - 1)];
		__sync_synchronize();
		m_Read = read + 1;

		return true;
	}
private:
	T m_Items[Size];
	volatile uint32 m_Read;
	volatile uint32 m_Write;
};

struct AudioCommandType_t
{
	enum Enum
	{
		Play,
		Stop,
		StopSound
	};
};

struct AudioCommand_t
{
	const int16* m_pSamples;
	uint64 m_Step;
	uint32 m_FrameCount;
	uint32 m_StartFrame;
	uint32 m_SoundIndex;
	uint32 m_ChannelIndex;
	uint16 m_ChannelCount;
	uint8 m_Type;
};

struct AudioEventType_t
{
	enum Enum
	{
		ChannelFinished,
		SoundReleased
	};
};

struct AudioEvent_t
{
	uint32 m_Index;
	uint8 m_Type;
};

struct SoundState_t
{
	enum Enum
	{
		Free,
		Loaded,
		Unloading
	};
};

struct SoundSlot_t
{
	Sound_t m_Sound;
	stb_vorbis* m_pDecoder;
	uint8* m_pData;
	uint32 m_DecodeStart;
	volatile uint32 m_DecodedEnd;
	uint16 m_Generation;
	uint8 m_State;
	uint8 m_ZeroDecodes;

	SoundSlot_t() :
		m_pDecoder(CF_NULL),
		m_pData(CF_NULL),
		m_DecodeStart(0),
		m_DecodedEnd(0),
		m_Generation(0),
		m_State(SoundState_t::Free),
		m_ZeroDecodes(0)
	{
	}
};

struct ChannelSlot_t
{
	uint32 m_SampleRate;
	uint32 m_SoundIndex;
	uint16 m_Generation;
	bool m_IsActive;
};

struct MixerChannel_t
{
	const int16* m_pSamples;
	uint64 m_Position;
	uint64 m_Step;
	uint32 m_FrameCount;
	uint32 m_SoundIndex;
	uint16 m_ChannelCount;
	bool m_IsPlaying;
};

class CAudioMixer
{
public:
	CAudioMixer();
	~CAudioMixer();

	void Shutdown();

	SoundHandle LoadSound(const char* pPath);
	void UnloadSound(SoundHandle hSound);
	SoundInstanceHandle PlaySound(SoundHandle hSound, float64 seconds);
	void StopSound(SoundInstanceHandle hInstance);
	bool IsSoundPlaying(SoundInstanceHandle hInstance) const;
	float64 GetSoundTime(SoundInstanceHandle hInstance) const;
	void SetSoundPaused(SoundInstanceHandle hInstance, bool isPaused);
	void SetSoundVolume(SoundInstanceHandle hInstance, float32 volume);
	void Update();

	void Mix(float32* pOutput, uint32 frameCount);
private:
	CAudioMixer(const CAudioMixer&);
	CAudioMixer& operator=(const CAudioMixer&);

	SoundSlot_t* GetSoundSlot(SoundHandle hSound);
	const ChannelSlot_t* GetChannelSlot(SoundInstanceHandle hInstance) const;
	void DecodeSound(SoundSlot_t& slot, uint32 frameCount);
	void ReleaseSound(SoundSlot_t& slot);
	bool MixChannel(
		MixerChannel_t& channel,
		uint32 decodedEnd,
		float32 volume,
		float32* pOutput,
		uint32 frameCount);
	void StopChannel(uint32 channelIndex);
	void ProcessCommands();

	CAudioQueue<AudioCommand_t, s_AudioCommandCount> m_Commands;
	SoundSlot_t m_Sounds[s_AudioMaxSounds];
	CAudioQueue<AudioEvent_t, s_AudioEventCount> m_Events;
	MixerChannel_t m_MixerChannels[s_AudioMaxChannels];
	ChannelSlot_t m_Channels[s_AudioMaxChannels];
	volatile uint32 m_ChannelFrames[s_AudioMaxChannels];
	volatile bool m_ChannelPaused[s_AudioMaxChannels];
	volatile float32 m_ChannelVolume[s_AudioMaxChannels];
};

#endif // AUDIO_MIXER_H
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "audio/Vorbis.h"
#include "audio/Sound.h"
#include "utils/UtlMemory.h"
#include "dbg.h"

#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include <stb/stb_vorbis.c>

stb_vorbis* OpenVorbis(const uint8* pData, uint32 size, Sound_t& outSound)
{
	int32 error = 0;
	stb_vorbis* pVorbis = stb_vorbis_open_memory(
		pData,
		int32(size),
		&error,
		CF_NULL);
	if (!pVorbis)
	{
		Error("[OpenVorbis] Failed to open stream: %d\n", error);

		return CF_NULL;
	}

	const stb_vorbis_info info = stb_vorbis_get_info(pVorbis);
	const uint32 frameCount = stb_vorbis_stream_length_in_samples(pVorbis);
	if (info.channels < 1 || info.channels > 2 || frameCount == 0)
	{
		Error(
			"[OpenVorbis] Unsupported stream: %d channels, %u frames\n",
			info.channels,
			frameCount);

		stb_vorbis_close(pVorbis);

		return CF_NULL;
	}

	const uint32 sampleCount = frameCount * uint32(info.channels);
	int16* pSamples = static_cast<int16*>(
		CUtlMemory::Alloc(sampleCount * sizeof(int16)));
	if (!pSamples)
	{
		Error("[OpenVorbis] Failed to allocate %u samples\n", sampleCount);

		stb_vorbis_close(pVorbis);

		return CF_NULL;
	}

	outSound.m_pSamples = pSamples;
	outSound.m_FrameCount = frameCount;
	outSound.m_SampleRate = info.sample_rate;
	outSound.m_ChannelCount = uint16(info.channels);

	return pVorbis;
}

uint32 DecodeVorbis(
	stb_vorbis* pVorbis,
	Sound_t& sound,
	uint32 firstFrame,
	uint32 frameCount)
{
	const int32 decodedFrames = stb_vorbis_get_samples_short_interleaved(
		pVorbis,
		sound.m_ChannelCount,
		sound.m_pSamples + firstFrame * sound.m_ChannelCount,
		int32(frameCount * sound.m_ChannelCount));

	return decodedFrames > 0 ? uint32(decodedFrames) : 0;
}

bool SeekVorbis(stb_vorbis* pVorbis, uint32 frame)
{
	if (frame == 0)
	{
		stb_vorbis_seek_start(pVorbis);

		return true;
	}

	return stb_vorbis_seek(pVorbis, frame) != 0;
}

void CloseVorbis(stb_vorbis* pVorbis)
{
	stb_vorbis_close(pVorbis);
}
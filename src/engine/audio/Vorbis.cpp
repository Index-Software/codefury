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

bool DecodeVorbis(const uint8* pData, uint32 size, Sound_t& outSound)
{
	int32 error = 0;
	stb_vorbis* pVorbis = stb_vorbis_open_memory(
		pData,
		int32(size),
		&error,
		CF_NULL);
	if (!pVorbis)
	{
		Error("[DecodeVorbis] Failed to open stream: %d\n", error);

		return false;
	}

	const stb_vorbis_info info = stb_vorbis_get_info(pVorbis);
	const uint32 frameCount = stb_vorbis_stream_length_in_samples(pVorbis);
	if (info.channels < 1 || info.channels > 2 || frameCount == 0)
	{
		Error(
			"[DecodeVorbis] Unsupported stream: %d channels, %u frames\n",
			info.channels,
			frameCount);

		stb_vorbis_close(pVorbis);

		return false;
	}

	const uint32 sampleCount = frameCount * uint32(info.channels);
	int16* pSamples = static_cast<int16*>(
		CUtlMemory::Alloc(sampleCount * sizeof(int16)));
	if (!pSamples)
	{
		Error("[DecodeVorbis] Failed to allocate %u samples\n", sampleCount);

		stb_vorbis_close(pVorbis);

		return false;
	}

	const int32 decodedFrames = stb_vorbis_get_samples_short_interleaved(
		pVorbis,
		info.channels,
		pSamples,
		int32(sampleCount));
	stb_vorbis_close(pVorbis);

	if (decodedFrames <= 0)
	{
		Error("[DecodeVorbis] Failed to decode stream\n");

		CUtlMemory::Free(pSamples);

		return false;
	}

	outSound.m_pSamples = pSamples;
	outSound.m_FrameCount = uint32(decodedFrames);
	outSound.m_SampleRate = info.sample_rate;
	outSound.m_ChannelCount = uint16(info.channels);

	return true;
}
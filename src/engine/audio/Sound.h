//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SOUND_H
#define SOUND_H

#pragma once

#include "platform.h"

struct Sound_t
{
	int16* m_pSamples;
	uint32 m_FrameCount;
	uint32 m_SampleRate;
	uint16 m_ChannelCount;

	Sound_t() :
		m_pSamples(CF_NULL),
		m_FrameCount(0),
		m_SampleRate(0),
		m_ChannelCount(0)
	{
	}
};

#endif // SOUND_H
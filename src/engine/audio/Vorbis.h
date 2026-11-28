//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef VORBIS_H
#define VORBIS_H

#pragma once

#include "platform.h"

struct Sound_t;
struct stb_vorbis;

stb_vorbis* OpenVorbis(const uint8* pData, uint32 size, Sound_t& outSound);
uint32 DecodeVorbis(
	stb_vorbis* pVorbis,
	Sound_t& sound,
	uint32 firstFrame,
	uint32 frameCount);
bool SeekVorbis(stb_vorbis* pVorbis, uint32 frame);
void CloseVorbis(stb_vorbis* pVorbis);

#endif // VORBIS_H
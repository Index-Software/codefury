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

bool DecodeVorbis(const uint8* pData, uint32 size, Sound_t& outSound);

#endif // VORBIS_H
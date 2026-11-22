/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IBL_UTILITIES_H
#define IBL_UTILITIES_H

#pragma once

#include "platform.h"
#include <math.h>

static inline void hammersley(
	uint32 index,
	float32 invCount,
	float32& outU,
	float32& outV)
{
	const float32 tof = 0.5f / float32(0x80000000u);
	uint32 bits = index;
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x55555555u) << 1) | ((bits & 0xAAAAAAAAu) >> 1);
	bits = ((bits & 0x33333333u) << 2) | ((bits & 0xCCCCCCCCu) >> 2);
	bits = ((bits & 0x0F0F0F0Fu) << 4) | ((bits & 0xF0F0F0F0u) >> 4);
	bits = ((bits & 0x00FF00FFu) << 8) | ((bits & 0xFF00FF00u) >> 8);
	outU = float32(index) * invCount;
	outV = float32(bits) * tof;
}

static inline float32 log4(float32 value)
{
	return 0.5f * log2f(value);
}

#endif // IBL_UTILITIES_H
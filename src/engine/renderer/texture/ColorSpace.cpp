//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/texture/ColorSpace.h"
#include <math.h>

float32 CColorSpace::SRGBToLinear(float32 value)
{
	if (value <= 0.04045f)
	{
		return value / 12.92f;
	}

	return powf((value + 0.055f) / 1.055f, 2.4f);
}

float32 CColorSpace::LinearToSRGB(float32 value)
{
	if (value <= 0.0031308f)
	{
		return value * 12.92f;
	}

	return 1.055f * powf(value, 1.0f / 2.4f) - 0.055f;
}
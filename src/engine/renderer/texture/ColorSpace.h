//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef COLORSPACE_H
#define COLORSPACE_H

#pragma once

#include "platform.h"

class CColorSpace
{
public:
	static float32 SRGBToLinear(float32 value);
	static float32 LinearToSRGB(float32 value);
};

#endif // COLORSPACE_H
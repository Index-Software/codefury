//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "utils/UniqueID.h"
#include <time.h>

uint64 CUniqueID::Generate()
{
	static uint32 s_Counter = 0;
	const uint64 now = static_cast<uint64>(time(CF_NULL));

	return (now << 32) | static_cast<uint64>(s_Counter++);
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#pragma once

#include "platform.h"

class CStringUtils
{
public:
	static bool GetLine(const char*& pText, char* pOutLine, uint32 lineSize);
};

#endif // STRING_UTILS_H
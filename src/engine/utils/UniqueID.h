//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef UNIQUE_ID_H
#define UNIQUE_ID_H

#pragma once

#include "platform.h"

class CUniqueID
{
public:
	static uint64 Generate();
};

#endif // UNIQUE_ID_H
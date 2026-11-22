//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENVIRONMENTPREFILTER_H
#define ENVIRONMENTPREFILTER_H

#pragma once

#include "platform.h"
#include "utils/UtlVector.h"

struct EnvironmentData_t;

class CEnvironmentPrefilter
{
public:
	static bool Prefilter(
		const CUtlVector< CUtlVector<uint8> >& faces,
		int32 faceSize,
		EnvironmentData_t& outData);
};

#endif // ENVIRONMENTPREFILTER_H
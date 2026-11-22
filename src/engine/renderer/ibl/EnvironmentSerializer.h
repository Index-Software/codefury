//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENVIRONMENTSERIALIZER_H
#define ENVIRONMENTSERIALIZER_H

#pragma once

#include "platform.h"
#include "utils/UtlVector.h"

struct EnvironmentData_t;

class CEnvironmentSerializer
{
public:
	static bool Serialize(
		const EnvironmentData_t& data,
		CUtlVector<uint8>& outBytes);
	static bool Deserialize(
		const CUtlVector<uint8>& bytes,
		EnvironmentData_t& outData);
};

#endif // ENVIRONMENTSERIALIZER_H
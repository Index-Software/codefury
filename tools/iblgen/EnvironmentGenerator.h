//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENVIRONMENTGENERATOR_H
#define ENVIRONMENTGENERATOR_H

#pragma once

#include "platform.h"

class CEnvironmentGenerator
{
public:
	static bool Generate(
		const char* pSourcePath,
		const char* pFaceSize,
		const char* pOutputPath);
	static bool GenerateDFG(
		const char* pSize,
		const char* pOutputPath);
};

#endif // ENVIRONMENTGENERATOR_H
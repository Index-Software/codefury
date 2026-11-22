//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef RADIX_SORT_H
#define RADIX_SORT_H

#pragma once

#include "platform.h"
#include "utils/UtlVector.h"

class CRadixSort
{
public:
	void SortIndices(
		const float32* pKeys,
		int32* pOutIndices,
		int32 count,
		bool isDescending);
private:
	CUtlVector<uint32> m_Keys[2];
	CUtlVector<int32> m_Indices[2];
};

#endif // RADIX_SORT_H
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "utils/RadixSort.h"
#include <string.h>

static const int32 s_Buckets = 256;
static const int32 s_Mask = s_Buckets - 1;

// https://stereopsis.com/radix.html
static uint32 FloatFlip(uint32 key)
{
	return (key & 0x80000000) ? (~key) : (key | 0x80000000);
}

void CRadixSort::SortIndices(
	const float32* pKeys,
	int32* pOutIndices,
	int32 count,
	bool isDescending)
{
	for (int32 i = 0; i < 2; i++)
	{
		m_Keys[i].SetCount(count);
		m_Indices[i].SetCount(count);
	}

	uint32* pSrc = m_Keys[0].Base();
	uint32* pDst = m_Keys[1].Base();
	int32* pSrcIdx = m_Indices[0].Base();
	int32* pDstIdx = m_Indices[1].Base();

	const uint32 invertMask = isDescending ? 0xFFFFFFFF : 0;
	for (int32 i = 0; i < count; i++)
	{
		pSrcIdx[i] = i;
		pSrc[i] =
			FloatFlip(reinterpret_cast<const uint32&>(pKeys[i])) ^ invertMask;
	}

	int32 hist[s_Buckets];
	int32 prefix[s_Buckets];

	for (int32 pass = 0; pass < 4; pass++)
	{
		int32 shift = pass * 8;
		memset(hist, 0, sizeof(hist));

		for (int32 i = 0; i < count; i++)
		{
			hist[(pSrc[i] >> shift) & s_Mask]++;
		}

		prefix[0] = 0;
		for (int32 i = 1; i < s_Buckets; i++)
		{
			prefix[i] = prefix[i - 1] + hist[i - 1];
		}

		for (int32 i = 0; i < count; i++)
		{
			int32 bucket = int32((pSrc[i] >> shift) & s_Mask);
			int32 pos = prefix[bucket]++;
			pDst[pos] = pSrc[i];
			pDstIdx[pos] = pSrcIdx[i];
		}

		uint32* pTmp = pSrc;
		pSrc = pDst;
		pDst = pTmp;

		int32* pTmpIdx = pSrcIdx;
		pSrcIdx = pDstIdx;
		pDstIdx = pTmpIdx;
	}

	for (int32 i = 0; i < count; i++)
	{
		pOutIndices[i] = pSrcIdx[i];
	}
}
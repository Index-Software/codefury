//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "utils/StringUtils.h"
#include <string.h>

bool CStringUtils::GetLine(const char*& pText, char* pOutLine, uint32 lineSize)
{
	if (!pText || !*pText)
	{
		return false;
	}

	const char* pLineEnd = strchr(pText, '\n');
	uint32 length = pLineEnd ? uint32(pLineEnd - pText) : uint32(strlen(pText));
	uint32 copyLength = length < lineSize ? length : lineSize - 1;

	memcpy(pOutLine, pText, copyLength);
	pOutLine[copyLength] = '\0';
	pText += length;
	if (pLineEnd)
	{
		pText++;
	}

	return true;
}
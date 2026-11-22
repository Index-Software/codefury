//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "utils/KeyValueLine.h"
#include <stdlib.h>
#include <string.h>

static bool IsSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool CopyToken(
	char* pOut,
	uint32 outSize,
	const char* pBegin,
	const char* pEnd)
{
	uint32 length = uint32(pEnd - pBegin);
	if (length == 0 || length >= outSize)
	{
		return false;
	}

	memcpy(pOut, pBegin, length);
	pOut[length] = '\0';

	return true;
}

CKeyValueLine::CKeyValueLine() :
	m_pInvalidKey(CF_NULL),
	m_PairCount(0)
{
	memset(m_Pairs, 0, sizeof(m_Pairs));
	memset(m_Tag, 0, sizeof(m_Tag));
}

bool CKeyValueLine::Parse(const char* pLine)
{
	m_Tag[0] = '\0';
	m_pInvalidKey = CF_NULL;
	m_PairCount = 0;

	const char* pCursor = pLine;
	while (IsSpace(*pCursor))
	{
		pCursor++;
	}

	if (*pCursor == '\0' || *pCursor == '#')
	{
		return true;
	}

	const char* pTokenEnd = pCursor;
	while (*pTokenEnd && !IsSpace(*pTokenEnd))
	{
		pTokenEnd++;
	}

	if (!CopyToken(m_Tag, sizeof(m_Tag), pCursor, pTokenEnd))
	{
		return false;
	}

	pCursor = pTokenEnd;

	while (true)
	{
		while (IsSpace(*pCursor))
		{
			pCursor++;
		}

		if (*pCursor == '\0')
		{
			return true;
		}

		const char* pKeyEnd = pCursor;
		while (*pKeyEnd && *pKeyEnd != '=' && !IsSpace(*pKeyEnd))
		{
			pKeyEnd++;
		}

		if (*pKeyEnd != '=' || m_PairCount >= 12)
		{
			return false;
		}

		Pair_t& pair = m_Pairs[m_PairCount];
		if (!CopyToken(pair.m_Key, sizeof(pair.m_Key), pCursor, pKeyEnd))
		{
			return false;
		}

		const char* pValueBegin = pKeyEnd + 1;
		const char* pValueEnd = pValueBegin;
		while (*pValueEnd && !IsSpace(*pValueEnd))
		{
			pValueEnd++;
		}

		if (!CopyToken(
			pair.m_Value,
			sizeof(pair.m_Value),
			pValueBegin,
			pValueEnd))
		{
			return false;
		}

		m_PairCount++;
		pCursor = pValueEnd;
	}
}

bool CKeyValueLine::IsEmpty() const
{
	return m_Tag[0] == '\0';
}

const char* CKeyValueLine::GetTag() const
{
	return m_Tag;
}

float32 CKeyValueLine::GetFloat(const char* pKey)
{
	const char* pValue = FindValue(pKey);
	if (pValue)
	{
		char* pEnd;
		double value = strtod(pValue, &pEnd);
		if (*pEnd == '\0')
		{
			return float32(value);
		}
	}

	if (!m_pInvalidKey)
	{
		m_pInvalidKey = pKey;
	}

	return 0.0f;
}

int32 CKeyValueLine::GetInt(const char* pKey)
{
	const char* pValue = FindValue(pKey);
	if (pValue)
	{
		char* pEnd;
		long value = strtol(pValue, &pEnd, 10);
		if (*pEnd == '\0')
		{
			return int32(value);
		}
	}

	if (!m_pInvalidKey)
	{
		m_pInvalidKey = pKey;
	}

	return 0;
}

const char* CKeyValueLine::GetInvalidKey() const
{
	return m_pInvalidKey;
}

const char* CKeyValueLine::FindValue(const char* pKey) const
{
	for (int32 i = 0; i < m_PairCount; i++)
	{
		if (strcmp(m_Pairs[i].m_Key, pKey) == 0)
		{
			return m_Pairs[i].m_Value;
		}
	}

	return CF_NULL;
}
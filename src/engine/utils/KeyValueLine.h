//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef KEY_VALUE_LINE_H
#define KEY_VALUE_LINE_H

#pragma once

#include "platform.h"

class CKeyValueLine
{
public:
	CKeyValueLine();

	bool Parse(const char* pLine);

	bool IsEmpty() const;
	const char* GetTag() const;

	float32 GetFloat(const char* pKey);
	int32 GetInt(const char* pKey);

	const char* GetInvalidKey() const;
private:
	struct Pair_t
	{
		char m_Value[32];
		char m_Key[24];
	};

	const char* FindValue(const char* pKey) const;

	Pair_t m_Pairs[12];
	char m_Tag[16];
	const char* m_pInvalidKey;
	int32 m_PairCount;
};

#endif // KEY_VALUE_LINE_H
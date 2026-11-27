//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CFILE_H
#define CFILE_H

#pragma once

#include "platform.h"
#include "WrenBind.h"
#include "utils/Utils.h"

class CFile
{
public:
	static const char* Read(const char* pPath) WREN_BIND_STATIC("read(_)")
	{
		static CFixedString s_Contents;
		s_Contents = CUtils::ReadFile(CFixedString(pPath));

		return s_Contents.c_str();
	}

	static bool Write(const char* pPath, const char* pContents)
		WREN_BIND_STATIC("write(_,_)")
	{
		return CUtils::WriteFile(CFixedString(pPath), CFixedString(pContents));
	}
};

#endif // CFILE_H
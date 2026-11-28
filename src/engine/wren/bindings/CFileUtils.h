//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CFILE_UTILS_H
#define CFILE_UTILS_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

class CFileUtils
{
public:
	static const char* Read(const char* pPath) WREN_BIND_STATIC("read(_)");
	static bool Write(const char* pPath, const char* pContents)
		WREN_BIND_STATIC("write(_,_)");
};

#else // WREN_BIND_CODEGEN

#include "utils/FileUtils.h"

#endif // !WREN_BIND_CODEGEN

#endif // CFILE_UTILS_H
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#pragma once

class CFileUtils
{
public:
	static const char* Read(const char* pPath);
	static bool Write(const char* pPath, const char* pContents);
};

#endif // FILE_UTILS_H
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "utils/FileUtils.h"
#include "utils/Utils.h"

const char* CFileUtils::Read(const char* pPath)
{
	static CFixedString s_Contents;
	s_Contents = CUtils::ReadFile(CFixedString(pPath));

	return s_Contents.c_str();
}

bool CFileUtils::Write(const char* pPath, const char* pContents)
{
	return CUtils::WriteFile(CFixedString(pPath), CFixedString(pContents));
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "application/Application.h"
#include "display/CDisplay.h"

Platform_t::Enum CApplication::GetPlatform()
{
#ifdef PLATFORM_PS3
	return Platform_t::PS3;
#else // PLATFORM_PS3
	return Platform_t::PC;
#endif // !PLATFORM_PS3
}

void CApplication::Quit()
{
	CDisplay::CloseWindow();
}
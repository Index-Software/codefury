//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include "platform.h"
#include "WrenBind.h"

struct Platform_t
{
	enum Enum
	{
		PC,
		PS3
	};
};

class CApplication
{
public:
	static Platform_t::Enum GetPlatform() WREN_BIND_STATIC("getPlatform()");
	static void Quit() WREN_BIND_STATIC("quit()");
};

#endif // APPLICATION_H
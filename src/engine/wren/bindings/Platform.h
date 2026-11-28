//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef PLATFORM_H
#define PLATFORM_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct Platform_t
{
	static int32 GetPC() WREN_BIND_STATIC("PC=PC");
	static int32 GetPS3() WREN_BIND_STATIC("PS3=PS3");
};

#else // WREN_BIND_CODEGEN

#include "application/Application.h"

#endif // !WREN_BIND_CODEGEN

#endif // PLATFORM_H
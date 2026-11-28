//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef INPUT_MODS_H
#define INPUT_MODS_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct InputMods_t
{
	static int32 GetNone() WREN_BIND_STATIC("None=None");
	static int32 GetShift() WREN_BIND_STATIC("Shift=Shift");
	static int32 GetControl() WREN_BIND_STATIC("Control=Control");
	static int32 GetAlt() WREN_BIND_STATIC("Alt=Alt");
	static int32 GetSuper() WREN_BIND_STATIC("Super=Super");
};

#else // WREN_BIND_CODEGEN

#include "input/InputActions.h"

#endif // !WREN_BIND_CODEGEN

#endif // INPUT_MODS_H
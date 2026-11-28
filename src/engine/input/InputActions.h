//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef INPUTACTIONS_H
#define INPUTACTIONS_H

#pragma once

#include "platform.h"
#include "WrenBind.h"

class IInput;

struct InputMods_t
{
	enum Enum
	{
		None = 0,
		Shift = 1 << 0,
		Control = 1 << 1,
		Alt = 1 << 2,
		Super = 1 << 3
	};
};

class CInputActions
{
public:
	static void SetInput(IInput* pInput);

	static bool Bind(const char* pName, int32 key, int32 mods)
		WREN_BIND_STATIC("bind(_,_,_)");
	static bool SetBinding(
		const char* pName,
		int32 index,
		int32 key,
		int32 mods) WREN_BIND_STATIC("setBinding(_,_,_,_)");
	static void ClearBindings(const char* pName)
		WREN_BIND_STATIC("clearBindings(_)");
	static int32 GetBindingCount(const char* pName)
		WREN_BIND_STATIC("getBindingCount(_)");
	static int32 GetBindingKey(const char* pName, int32 index)
		WREN_BIND_STATIC("getBindingKey(_,_)");
	static int32 GetBindingMods(const char* pName, int32 index)
		WREN_BIND_STATIC("getBindingMods(_,_)");

	static bool IsDown(const char* pName) WREN_BIND_STATIC("isDown(_)");
	static bool WasPressed(const char* pName)
		WREN_BIND_STATIC("wasPressed(_)");
	static bool WasReleased(const char* pName)
		WREN_BIND_STATIC("wasReleased(_)");
};

#endif // INPUTACTIONS_H
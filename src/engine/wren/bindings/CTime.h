//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CTIME_H
#define CTIME_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

class CTime
{
public:
	static float64 GetTime() WREN_BIND_STATIC("getTime()");
	static float32 GetDeltaTime() WREN_BIND_STATIC("getDeltaTime()");
	static uint64 GetFrameCount() WREN_BIND_STATIC("getFrameCount()");
	static float64 GetTimeSinceStartup()
		WREN_BIND_STATIC("getTimeSinceStartup()");
};

#else // WREN_BIND_CODEGEN

#include "utils/Time.h"

#endif // !WREN_BIND_CODEGEN

#endif // CTIME_H
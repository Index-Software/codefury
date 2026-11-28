//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CDISPLAY_H
#define CDISPLAY_H

#pragma once

#include "platform.h"
#include "WrenBind.h"

class IWindow;

class CDisplay
{
public:
	static void SetVSync(bool isEnabled) WREN_BIND_STATIC("setVSync(_)");
	static void SetFrameLimit(int32 fps) WREN_BIND_STATIC("setFrameLimit(_)");
	static void SetFullscreen(
		bool isFullscreen) WREN_BIND_STATIC("setFullscreen(_)");
	static bool IsFullscreen() WREN_BIND_STATIC("isFullscreen()");
	static void SetWindow(IWindow* pWindow);
	static void WaitForFrame();
	static void CloseWindow();
};

#endif // CDISPLAY_H
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ANCHOR_H
#define ANCHOR_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct Anchor_t
{
	static int32 GetTopLeft() WREN_BIND_STATIC("TopLeft=TopLeft");
	static int32 GetTop() WREN_BIND_STATIC("Top=Top");
	static int32 GetTopRight() WREN_BIND_STATIC("TopRight=TopRight");
	static int32 GetLeft() WREN_BIND_STATIC("Left=Left");
	static int32 GetCenter() WREN_BIND_STATIC("Center=Center");
	static int32 GetRight() WREN_BIND_STATIC("Right=Right");
	static int32 GetBottomLeft() WREN_BIND_STATIC("BottomLeft=BottomLeft");
	static int32 GetBottom() WREN_BIND_STATIC("Bottom=Bottom");
	static int32 GetBottomRight() WREN_BIND_STATIC("BottomRight=BottomRight");
};

#endif // WREN_BIND_CODEGEN

#endif // ANCHOR_H
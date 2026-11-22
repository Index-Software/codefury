//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef FACE_ORIENTATION_H
#define FACE_ORIENTATION_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct FaceOrientation_t
{
	static int32 GetInwards() WREN_BIND_STATIC("Inwards=Inwards");
	static int32 GetOutwards() WREN_BIND_STATIC("Outwards=Outwards");
};

#endif // WREN_BIND_CODEGEN

#endif // FACE_ORIENTATION_H
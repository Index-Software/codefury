//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef TEXTURE_WRAP_MODE_H
#define TEXTURE_WRAP_MODE_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct TextureWrapMode_t
{
	static int32 GetRepeat() WREN_BIND_STATIC("Repeat=Repeat");
	static int32 GetMirroredRepeat()
		WREN_BIND_STATIC("MirroredRepeat=MirroredRepeat");
	static int32 GetClampToEdge() WREN_BIND_STATIC("ClampToEdge=ClampToEdge");
};

#endif // WREN_BIND_CODEGEN

#endif // TEXTURE_WRAP_MODE_H
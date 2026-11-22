//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SHADER_TYPE_H
#define SHADER_TYPE_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct ShaderType_t
{
	static int32 GetDefault() WREN_BIND_STATIC("Default=Default");
	static int32 GetSkybox() WREN_BIND_STATIC("Skybox=Skybox");
};

#endif // WREN_BIND_CODEGEN

#endif // SHADER_TYPE_H
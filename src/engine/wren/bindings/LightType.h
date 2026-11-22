//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef LIGHT_TYPE_H
#define LIGHT_TYPE_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct LightType_t
{
	static int32 GetPoint() WREN_BIND_STATIC("Point=Point");
	static int32 GetDirectional() WREN_BIND_STATIC("Directional=Directional");
};

#endif // WREN_BIND_CODEGEN

#endif // LIGHT_TYPE_H
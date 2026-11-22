//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SHADER_BINDINGS_H
#define SHADER_BINDINGS_H

#pragma once

#include "ShaderConstants.h"

#if defined(__cplusplus) || defined(__SLANG__)
	#define CF_BINDING_IBL_SPECULAR 7
	#define CF_BINDING_IBL_DFG 8
	#define CF_BINDING_SCENE 9
	#define CF_BINDING_LIGHTING 10
	#define CF_BINDING_CAMERA 11
	#define CF_BINDING_MODEL 12
	#define CF_BINDING_GLOBAL_ENV_MAP 13
	#define CF_BINDING_SHADOW_MAP 14
	#define CF_BINDING_POINT_SHADOW_MAP 15
#else // defined(__cplusplus) || defined(__SLANG__)
	#define CF_BINDING_IBL_SPECULAR TEXUNIT7
	#define CF_BINDING_IBL_DFG TEXUNIT8
	#define CF_BINDING_SCENE register(c9)
	#define CF_BINDING_LIGHTING register(c10)
	#define CF_BINDING_CAMERA register(c11)
	#define CF_BINDING_MODEL register(c12)
	#define CF_BINDING_GLOBAL_ENV_MAP TEXUNIT13
	#define CF_BINDING_SHADOW_MAP TEXUNIT14
	#define CF_BINDING_POINT_SHADOW_MAP TEXUNIT15
#endif // !defined(__cplusplus) && !defined(__SLANG__)

#endif // SHADER_BINDINGS_H
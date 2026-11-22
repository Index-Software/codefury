//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CF_PLATFORM_H
#define CF_PLATFORM_H

#pragma once

#include "../thirdparty/gcmgl/public/platform.h"

#if defined(__cplusplus) && __cplusplus >= 201103L
	#define CF_MODERN_CPP 1
#endif // defined(__cplusplus) && __cplusplus >= 201103L

#ifdef CF_MODERN_CPP
	#define CF_NULL nullptr
	#define CF_OVERRIDE override
#else // CF_MODERN_CPP
	#define CF_NULL 0
	#define CF_OVERRIDE
#endif // !CF_MODERN_CPP

#endif // CF_PLATFORM_H
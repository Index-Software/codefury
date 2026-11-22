//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CCOLOR_H
#define CCOLOR_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct CColor
{
	float32 m_R;
	float32 m_G;
	float32 m_B;
	float32 m_A;

	CColor() WREN_CONSTRUCTOR("init new()");
	CColor(
		float32 r,
		float32 g,
		float32 b,
		float32 a) WREN_CONSTRUCTOR("init new(_,_,_,_)");

	float32 GetR() const WREN_BIND("r=m_R");
	float32 GetG() const WREN_BIND("g=m_G");
	float32 GetB() const WREN_BIND("b=m_B");
	float32 GetA() const WREN_BIND("a=m_A");

	static CColor GetBlack() WREN_BIND_STATIC("Black=Black");
	static CColor GetWhite() WREN_BIND_STATIC("White=White");
	static CColor GetRed() WREN_BIND_STATIC("Red=Red");
	static CColor GetGreen() WREN_BIND_STATIC("Green=Green");
	static CColor GetBlue() WREN_BIND_STATIC("Blue=Blue");

	static uint32 PackColor(const CColor&) WREN_BIND_STATIC("packColor(_)");
	static uint32 PackRGBA(const CColor&) WREN_BIND_STATIC("packRGBA(_)");
	static uint32 PackARGB(const CColor&) WREN_BIND_STATIC("packARGB(_)");
	static uint32 PackABGR(const CColor&) WREN_BIND_STATIC("packABGR(_)");
};

#endif // WREN_BIND_CODEGEN

#endif // CCOLOR_H
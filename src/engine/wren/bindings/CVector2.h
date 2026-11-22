//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CVECTOR2_H
#define CVECTOR2_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct CVector2
{
	CVector2(float32 x, float32 y) WREN_CONSTRUCTOR("init new(_,_)");

	float32 GetX() const WREN_BIND("x=m_X");
	float32 GetY() const WREN_BIND("y=m_Y");

	float32 Length() const WREN_BIND("length");
	float32 LengthSq() const WREN_BIND("lengthSq");
	float32 Distance(const CVector2&) const WREN_BIND("distance(_)");
	float32 DistanceSq(const CVector2&) const WREN_BIND("distanceSq(_)");

	CVector2 operator+(const CVector2&) const WREN_BIND("add(_)");
	CVector2 operator-(const CVector2&) const WREN_BIND("subtract(_)");
	CVector2 operator*(const CVector2&) const WREN_BIND("multiply(_)");
	CVector2 operator/(const CVector2&) const WREN_BIND("divide(_)");
	CVector2 operator*(float32) const WREN_BIND("multiplyScalar(_)");
	CVector2 operator/(float32) const WREN_BIND("divideScalar(_)");
	bool operator==(const CVector2&) const WREN_BIND("equals(_)");
	bool operator!=(const CVector2&) const WREN_BIND("notEquals(_)");
};

#endif // WREN_BIND_CODEGEN

#endif // CVECTOR2_H
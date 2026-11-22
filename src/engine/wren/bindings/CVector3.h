//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CVECTOR3_H
#define CVECTOR3_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct CVector3
{
	CVector3(float32 x, float32 y, float32 z)
		WREN_CONSTRUCTOR("init new(_,_,_)");

	float32 GetX() const WREN_BIND("x=m_X");
	float32 GetY() const WREN_BIND("y=m_Y");
	float32 GetZ() const WREN_BIND("z=m_Z");

	float32 Length() const WREN_BIND("length");
	float32 LengthSq() const WREN_BIND("lengthSq");
	float32 LengthXY() const WREN_BIND("lengthXY");
	float32 Distance(const CVector3&) const WREN_BIND("distance(_)");
	float32 DistanceSq(const CVector3&) const WREN_BIND("distanceSq(_)");
	float32 Dot(const CVector3&) const WREN_BIND("dot(_)");
	CVector3 Cross(const CVector3&) const WREN_BIND("cross(_)");

	CVector3 operator+(const CVector3&) const WREN_BIND("add(_)");
	CVector3 operator-(const CVector3&) const WREN_BIND("subtract(_)");
	CVector3 operator*(const CVector3&) const WREN_BIND("multiply(_)");
	CVector3 operator/(const CVector3&) const WREN_BIND("divide(_)");
	CVector3 operator*(float32) const WREN_BIND("multiplyScalar(_)");
	CVector3 operator/(float32) const WREN_BIND("divideScalar(_)");
	bool operator==(const CVector3&) const WREN_BIND("equals(_)");
	bool operator!=(const CVector3&) const WREN_BIND("notEquals(_)");
};

#endif // WREN_BIND_CODEGEN

#endif // CVECTOR3_H
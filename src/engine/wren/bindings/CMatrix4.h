//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CMATRIX4_H
#define CMATRIX4_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct CVector3;

struct CMatrix4
{
	CMatrix4() WREN_CONSTRUCTOR("init new()");

	CMatrix4 operator*(const CMatrix4&) const WREN_BIND("multiply(_)");
	CMatrix4 operator+(const CMatrix4&) const WREN_BIND("add(_)");
	CMatrix4 operator-(const CMatrix4&) const WREN_BIND("subtract(_)");
	bool operator==(const CMatrix4&) const WREN_BIND("equals(_)");
	bool operator!=(const CMatrix4&) const WREN_BIND("notEquals(_)");
	CMatrix4 ToTransformMatrix(
		const CVector3&,
		const CVector3&) const WREN_BIND("toTransformMatrix(_,_)");
};

#endif // WREN_BIND_CODEGEN

#endif // CMATRIX4_H
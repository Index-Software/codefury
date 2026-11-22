//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CMATHS_H
#define CMATHS_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"
#include "CVector3.h"
#include "CMatrix4.h"

struct EquirectUV_t
{
	float32 m_U;
	float32 m_V;
};

struct CMaths
{
	static float32 Pi() WREN_BIND_STATIC("Pi=PI");
	static float32 TwoPi() WREN_BIND_STATIC("TwoPi=TwoPI");
	static float32 Deg2Rad() WREN_BIND_STATIC("Deg2Rad=Deg2Rad");
	static float32 Rad2Deg() WREN_BIND_STATIC("Rad2Deg=Rad2Deg");

	static float32 Sqrt(float32) WREN_BIND_STATIC("sqrt(_)");
	static float32 Tan(float32) WREN_BIND_STATIC("tan(_)");
	static float32 Cos(float32) WREN_BIND_STATIC("cos(_)");
	static float32 Sin(float32) WREN_BIND_STATIC("sin(_)");
	static float32 ATan2(float32, float32) WREN_BIND_STATIC("atan2(_,_)");
	static float32 ASin(float32) WREN_BIND_STATIC("asin(_)");
	static float32 Abs(float32) WREN_BIND_STATIC("abs(_)");
	static float32 Min(float32, float32) WREN_BIND_STATIC("min(_,_)");
	static float32 Max(float32, float32) WREN_BIND_STATIC("max(_,_)");
	static float32 Clamp(
		float32,
		float32,
		float32) WREN_BIND_STATIC("clamp(_,_,_)");

	static CVector3 Normalise(const CVector3&) WREN_BIND_STATIC("normalise(_)");
	static CVector3 Cross(
		const CVector3&,
		const CVector3&) WREN_BIND_STATIC("cross(_,_)");
	static float32 Dot(
		const CVector3&,
		const CVector3&) WREN_BIND_STATIC("dot(_,_)");

	static CMatrix4 Perspective(
		float32,
		float32,
		float32,
		float32) WREN_BIND_STATIC("perspective(_,_,_,_)");
	static CMatrix4 Orthographic(
		float32,
		float32,
		float32,
		float32,
		float32,
		float32) WREN_BIND_STATIC("orthographic(_,_,_,_,_,_)");
	static CMatrix4 LookAt(
		const CVector3&,
		const CVector3&,
		const CVector3&) WREN_BIND_STATIC("lookAt(_,_,_)");
	static CMatrix4 Translate(
		const CMatrix4&,
		const CVector3&) WREN_BIND_STATIC("translate(_,_)");
	static CMatrix4 Rotate(
		const CMatrix4&,
		float32,
		const CVector3&) WREN_BIND_STATIC("rotate(_,_,_)");
	static CMatrix4 Scale(
		const CMatrix4&,
		const CVector3&) WREN_BIND_STATIC("scale(_,_)");

	static EquirectUV_t DirectionToEquirect(
		const CVector3&) WREN_BIND_FWD(
			"directionToEquirect(_)",
			"WrenFwd_CMaths_DirectionToEquirect");
};

#endif // WREN_BIND_CODEGEN

#endif // CMATHS_H
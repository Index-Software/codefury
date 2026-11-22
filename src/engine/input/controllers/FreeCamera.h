//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef FREE_CAMERA_H
#define FREE_CAMERA_H

#pragma once

#include "mathsfury/Vector3.h"
#include "WrenBind.h"

class IInput;
class CCamera;

class CFreeCamera
{
public:
	CFreeCamera() WREN_CONSTRUCTOR("init new()");
	~CFreeCamera();

	void Update(
		CCamera& camera,
		IInput* pInput,
		float32 deltaTime) WREN_BIND("update(_,_)");

	void SetPosition(const CVector3& position) WREN_BIND("setPosition(_)");
	void SetRotation(const CVector3& rotation) WREN_BIND("setRotation(_)");
	void SetSpeed(float32 speed) WREN_BIND("setSpeed(_)");

	CVector3 GetPosition() const WREN_BIND("getPosition");
	CVector3 GetRotation() const WREN_BIND("getRotation");
private:
	CVector3 m_Position;
	float32 m_Yaw;
	float32 m_Pitch;
	float32 m_Speed;
};

#endif // FREE_CAMERA_H
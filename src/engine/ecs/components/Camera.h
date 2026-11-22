//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CAMERA_H
#define CAMERA_H

#pragma once

#include "mathsfury/Maths.h"
#include "ecs/EntityComponent.h"
#include "WrenBind.h"

class CCamera : public CEntityComponent
{
public:
	CCamera() WREN_CONSTRUCTOR("init new()") :
		m_Position(0.0f, 0.0f, 0.0f),
		m_Target(0.0f, 0.0f, 1.0f),
		m_Up(0.0f, 1.0f, 0.0f),
		m_FOV(45.0f),
		m_Aspect(1.0f),
		m_NearPlane(0.1f),
		m_FarPlane(100.0f)
	{
	}

	virtual ~CCamera() CF_OVERRIDE
	{
	}

	virtual const char* TypeName() const CF_OVERRIDE
	{
		return "CCamera";
	}

	void SetPerspective(
		float32 fov,
		float32 aspect,
		float32 nearPlane,
		float32 farPlane) WREN_BIND("setPerspective(_,_,_,_)");
	void SetPosition(const CVector3& position) WREN_BIND("setPosition(_)");
	void LookAt(
		const CVector3& target,
		const CVector3& up = CVector3(0.0f, 1.0f, 0.0f))
		WREN_BIND("lookAt(_,_)");
	void SetRotation(const CVector3& rotation) WREN_BIND("setRotation(_)");

	const CVector3& GetPosition() const WREN_BIND("getPosition")
	{
		return m_Position;
	}

	const CVector3& GetTarget() const WREN_BIND("getTarget")
	{
		return m_Target;
	}

	float32 GetAspect() const WREN_BIND("getAspect")
	{
		return m_Aspect;
	}

	float32 GetFOV() const WREN_BIND("getFOV")
	{
		return m_FOV;
	}

	CMatrix4 GetViewMatrix() const WREN_BIND("getViewMatrix");
	CMatrix4 GetProjectionMatrix() const WREN_BIND("getProjectionMatrix");
	CMatrix4 GetViewProjectionMatrix() const
		WREN_BIND("getViewProjectionMatrix");
private:
	CVector3 m_Position;
	CVector3 m_Target;
	CVector3 m_Up;

	float32 m_FOV;
	float32 m_Aspect;
	float32 m_NearPlane;
	float32 m_FarPlane;
};

#endif // CAMERA_H
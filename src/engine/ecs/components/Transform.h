//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef TRANSFORM_H
#define TRANSFORM_H

#pragma once

#include "ecs/EntityComponent.h"
#include "mathsfury/Maths.h"
#include "WrenBind.h"

class CTransform : public CEntityComponent
{
public:
	CTransform() WREN_CONSTRUCTOR("init new()") : m_Scale(1.0f, 1.0f, 1.0f)
	{
	}

	virtual const char* TypeName() const CF_OVERRIDE
	{
		return "CTransform";
	}

	CVector3 m_Position;
	CVector3 m_Rotation;
	CVector3 m_Scale;

	CMatrix4 GetMatrix() const WREN_BIND("getMatrix")
	{
		CMatrix4 rotX = CMaths::Rotate(
			CMatrix4(1.0f),
			m_Rotation.m_X,
			CVector3(1.0f, 0.0f, 0.0f));
		CMatrix4 rotY = CMaths::Rotate(
			CMatrix4(1.0f),
			m_Rotation.m_Y,
			CVector3(0.0f, 1.0f, 0.0f));
		CMatrix4 rotZ = CMaths::Rotate(
			CMatrix4(1.0f),
			m_Rotation.m_Z,
			CVector3(0.0f, 0.0f, 1.0f));

		CMatrix4 rotationMatrix = rotZ * rotY * rotX;

		return rotationMatrix.ToTransformMatrix(m_Position, m_Scale);
	}

	const CVector3& GetPosition() const WREN_BIND("getPosition")
	{
		return m_Position;
	}

	void SetPosition(const CVector3& v) WREN_BIND("setPosition(_)")
	{
		m_Position = v;
	}

	const CVector3& GetRotation() const WREN_BIND("getRotation")
	{
		return m_Rotation;
	}

	void SetRotation(const CVector3& v) WREN_BIND("setRotation(_)")
	{
		m_Rotation = v;
	}

	const CVector3& GetScale() const WREN_BIND("getScale")
	{
		return m_Scale;
	}

	void SetScale(const CVector3& v) WREN_BIND("setScale(_)")
	{
		m_Scale = v;
	}
};

#endif // TRANSFORM_H
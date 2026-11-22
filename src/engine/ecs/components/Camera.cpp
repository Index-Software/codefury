//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Camera.h"
#include <math.h>

void CCamera::SetPerspective(
	float32 fov,
	float32 aspect,
	float32 nearPlane,
	float32 farPlane)
{
	m_FOV = fov;
	m_Aspect = aspect;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
}

void CCamera::SetPosition(const CVector3& position)
{
	m_Position = position;
}

void CCamera::LookAt(const CVector3& target, const CVector3& up)
{
	m_Target = target;
	m_Up = up;
}

void CCamera::SetRotation(const CVector3& rotation)
{
	float32 pitchRad = rotation.m_X * CMaths::Deg2Rad;
	float32 yawRad = rotation.m_Y * CMaths::Deg2Rad;
	float32 rollRad = rotation.m_Z * CMaths::Deg2Rad;

	CVector3 forward;
	forward.m_X = cosf(yawRad) * cosf(pitchRad);
	forward.m_Y = sinf(pitchRad);
	forward.m_Z = sinf(yawRad) * cosf(pitchRad);
	forward = CMaths::Normalise(forward);

	CVector3 right = CMaths::Normalise(
		CMaths::Cross(forward, CVector3(0.0f, 1.0f, 0.0f)));

	CVector3 up = CMaths::Normalise(CMaths::Cross(right, forward));

	if (rollRad != 0.0f)
	{
		CMatrix4 rollMatrix = CMaths::Rotate(CMatrix4(1.0f), rollRad, forward);

		CVector4 upVector4(up.m_X, up.m_Y, up.m_Z, 0.0f);
		CVector4 upRotated = rollMatrix * upVector4;

		up = CMaths::Normalise(
			CVector3(upRotated.m_X, upRotated.m_Y, upRotated.m_Z));
	}

	CVector3 target = m_Position + forward;

	LookAt(target, up);
}

CMatrix4 CCamera::GetViewMatrix() const
{
	return CMaths::LookAt(m_Position, m_Target, m_Up);
}

CMatrix4 CCamera::GetProjectionMatrix() const
{
	return CMaths::Perspective(m_FOV, m_Aspect, m_NearPlane, m_FarPlane);
}

CMatrix4 CCamera::GetViewProjectionMatrix() const
{
	return GetProjectionMatrix() * GetViewMatrix();
}
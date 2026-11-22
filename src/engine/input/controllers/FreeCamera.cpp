//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "FreeCamera.h"
#include "ecs/components/Camera.h"
#include "input/Input.h"
#include "config/ConfigVar.h"
#include "utils/Time.h"
#include "mathsfury/Maths.h"
#include <math.h>

static CConfigVar s_clSensitivity(
	"cl_sensitivity",
	0.1f,
	ConfigVarArchive,
	0.0f,
	10.0f);

CFreeCamera::CFreeCamera()
	: m_Position(0.0f, 5.0f, 9.33f)
	, m_Yaw(-90.0f)
	, m_Pitch(0.0f)
	, m_Speed(20.0f)
{
}

CFreeCamera::~CFreeCamera()
{
}

void CFreeCamera::Update(CCamera& camera, IInput* pInput, float32 deltaTime)
{
	if (!pInput)
	{
		return;
	}

	float32 lookX = pInput->GetLookX();
	float32 lookY = pInput->GetLookY();
	float32 moveX = pInput->GetMoveX();
	float32 moveY = pInput->GetMoveY();

	m_Yaw += lookX * CMaths::Rad2Deg * s_clSensitivity.GetFloat();
	m_Pitch -= lookY * CMaths::Rad2Deg * s_clSensitivity.GetFloat();
	m_Pitch = CMaths::Clamp(m_Pitch, -89.0f, 89.0f);

	float32 cosYaw = cosf(m_Yaw * CMaths::Deg2Rad);
	float32 sinYaw = sinf(m_Yaw * CMaths::Deg2Rad);
	float32 cosPitch = cosf(m_Pitch * CMaths::Deg2Rad);
	float32 sinPitch = sinf(m_Pitch * CMaths::Deg2Rad);

	CVector3 forward(cosYaw * cosPitch, sinPitch, sinYaw * cosPitch);
	forward = CMaths::Normalise(forward);
	CVector3 right = CMaths::Normalise(
		CMaths::Cross(forward, CVector3(0.0f, 1.0f, 0.0f)));

	m_Position += right * moveX * m_Speed * deltaTime;
	m_Position += forward * moveY * m_Speed * deltaTime;

	camera.SetPosition(m_Position);
	camera.SetRotation(CVector3(m_Pitch, m_Yaw, 0.0f));
}

void CFreeCamera::SetPosition(const CVector3& position)
{
	m_Position = position;
}

void CFreeCamera::SetRotation(const CVector3& rotation)
{
	m_Pitch = rotation.m_X;
	m_Yaw = rotation.m_Y;
}

void CFreeCamera::SetSpeed(float32 speed)
{
	m_Speed = speed;
}

CVector3 CFreeCamera::GetPosition() const
{
	return m_Position;
}

CVector3 CFreeCamera::GetRotation() const
{
	return CVector3(m_Pitch, m_Yaw, 0.0f);
}
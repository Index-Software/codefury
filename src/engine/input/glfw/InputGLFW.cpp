//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "InputGLFW.h"
#include "utils/Time.h"
#include "utils/UtlMemory.h"
#include <GLFW/glfw3.h>
#include <string.h>
#include <new>

static const float32 s_LookScale = 0.002f;

CInputGLFW::CInputGLFW() :
	m_pWindow(CF_NULL),
	m_PreviousMouseX(0.0),
	m_PreviousMouseY(0.0),
	m_AccumulatedLookX(0.0f),
	m_AccumulatedLookY(0.0f),
	m_KeyEventCount(0),
	m_PendingKeyEventCount(0),
	m_IsMouseActive(false),
	m_HasMouseOrigin(false)
{
	memset(m_KeyState, 0, sizeof(m_KeyState));
	memset(m_LastKeyState, 0, sizeof(m_LastKeyState));
}

CInputGLFW::~CInputGLFW()
{
	Shutdown();
}

int32 CInputGLFW::Init()
{
	return 1;
}

void CInputGLFW::Shutdown()
{
	if (m_pWindow)
	{
		SetMouseActive(false);
	}
}

void CInputGLFW::Update()
{
	if (!m_pWindow)
	{
		return;
	}

	memcpy(m_LastKeyState, m_KeyState, sizeof(m_KeyState));
	for (int32 i = 0; i < m_PendingKeyEventCount; i++)
	{
		const KeyEvent_t& event = m_PendingKeyEvents[i];
		if (event.m_Key >= 0 && event.m_Key < s_MaxKeys)
		{
			m_KeyState[event.m_Key] = event.m_IsPress;
		}
	}

	memcpy(
		m_KeyEvents,
		m_PendingKeyEvents,
		m_PendingKeyEventCount * sizeof(KeyEvent_t));
	m_KeyEventCount = m_PendingKeyEventCount;
	m_PendingKeyEventCount = 0;

	m_AccumulatedLookX = 0.0f;
	m_AccumulatedLookY = 0.0f;

	if (m_IsMouseActive)
	{
		float64 x;
		float64 y;
		glfwGetCursorPos(m_pWindow, &x, &y);

		if (m_HasMouseOrigin)
		{
			m_AccumulatedLookX = static_cast<float32>(x - m_PreviousMouseX);
			m_AccumulatedLookY = static_cast<float32>(y - m_PreviousMouseY);
		}

		m_PreviousMouseX = x;
		m_PreviousMouseY = y;
		m_HasMouseOrigin = true;
	}
}

void CInputGLFW::SetWindow(void* pWindow)
{
	m_pWindow = static_cast<GLFWwindow*>(pWindow);
	if (m_pWindow)
	{
		glfwSetWindowUserPointer(m_pWindow, this);
		glfwSetKeyCallback(m_pWindow, OnKey);
		SetMouseActive(true);
	}
}

void CInputGLFW::SetMouseCaptured(bool isCaptured)
{
	if (m_pWindow)
	{
		SetMouseActive(isCaptured);
	}
}

bool CInputGLFW::GetMouseCaptured()
{
	return m_IsMouseActive;
}

float32 CInputGLFW::GetLookX()
{
	return m_AccumulatedLookX * s_LookScale;
}

float32 CInputGLFW::GetLookY()
{
	return m_AccumulatedLookY * s_LookScale;
}

float32 CInputGLFW::GetMoveX()
{
	if (!m_pWindow)
	{
		return 0.0f;
	}

	float32 moveX = 0.0f;
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		moveX -= 1.0f;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		moveX += 1.0f;
	}

	return moveX;
}

float32 CInputGLFW::GetMoveY()
{
	if (!m_pWindow)
	{
		return 0.0f;
	}

	float32 moveY = 0.0f;
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		moveY += 1.0f;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		moveY -= 1.0f;
	}

	return moveY;
}

bool CInputGLFW::IsKeyDown(int32 key)
{
	if (key < 0 || key >= s_MaxKeys)
	{
		return false;
	}

	return m_KeyState[key];
}

bool CInputGLFW::WasKeyPressed(int32 key)
{
	if (key < 0 || key >= s_MaxKeys)
	{
		return false;
	}

	return m_KeyState[key] && !m_LastKeyState[key];
}

bool CInputGLFW::WasKeyReleased(int32 key)
{
	if (key < 0 || key >= s_MaxKeys)
	{
		return false;
	}

	return !m_KeyState[key] && m_LastKeyState[key];
}

int32 CInputGLFW::GetKeyEventCount()
{
	return m_KeyEventCount;
}

int32 CInputGLFW::GetKeyEventKey(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return 0;
	}

	return m_KeyEvents[index].m_Key;
}

int32 CInputGLFW::GetKeyEventMods(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return 0;
	}

	return m_KeyEvents[index].m_Mods;
}

bool CInputGLFW::WasKeyEventPressed(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return false;
	}

	return m_KeyEvents[index].m_IsPress;
}

bool CInputGLFW::WasKeyEventReleased(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return false;
	}

	return !m_KeyEvents[index].m_IsPress;
}

float64 CInputGLFW::GetKeyEventTime(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return 0.0;
	}

	return m_KeyEvents[index].m_Time;
}

void CInputGLFW::OnKey(
	GLFWwindow* pWindow,
	int key,
	int scancode,
	int action,
	int mods)
{
	CInputGLFW* pInput =
		static_cast<CInputGLFW*>(glfwGetWindowUserPointer(pWindow));
	if (!pInput || action == GLFW_REPEAT
		|| pInput->m_PendingKeyEventCount >= s_MaxKeyEvents)
	{
		return;
	}

	KeyEvent_t& event =
		pInput->m_PendingKeyEvents[pInput->m_PendingKeyEventCount];
	pInput->m_PendingKeyEventCount++;

	event.m_Time = CTime::GetTimeSinceStartup();
	event.m_Key = key;
	event.m_Mods = mods;
	event.m_IsPress = (action == GLFW_PRESS);
}

void CInputGLFW::SetMouseActive(bool active)
{
	m_IsMouseActive = active;

	if (active && glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	else
	{
		glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	}

	glfwSetInputMode(
		m_pWindow,
		GLFW_CURSOR,
		active ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

	m_AccumulatedLookX = 0.0f;
	m_AccumulatedLookY = 0.0f;
	m_HasMouseOrigin = false;
}

IInput* CreateInput()
{
	void* pMemory = CUtlMemory::Alloc(sizeof(CInputGLFW));
	if (pMemory)
	{
		return new(pMemory) CInputGLFW();
	}

	return CF_NULL;
}
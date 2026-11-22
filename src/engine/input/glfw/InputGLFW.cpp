//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "InputGLFW.h"
#include "utils/UtlMemory.h"
#include <GLFW/glfw3.h>
#include <new>

static const float32 s_LookScale = 0.002f;

CInputGLFW::CInputGLFW() :
	m_pWindow(CF_NULL),
	m_PreviousMouseX(0.0),
	m_PreviousMouseY(0.0),
	m_AccumulatedLookX(0.0f),
	m_AccumulatedLookY(0.0f),
	m_IsMouseActive(false),
	m_HasMouseOrigin(false)
{
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
}

void CInputGLFW::Update()
{
	if (!m_pWindow)
	{
		return;
	}

	if (m_IsMouseActive && glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		SetMouseActive(false);
	}
	else if (!m_IsMouseActive &&
		glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		SetMouseActive(true);
	}

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
		SetMouseActive(true);
	}
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
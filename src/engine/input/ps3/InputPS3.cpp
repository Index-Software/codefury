//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "InputPS3.h"
#include "utils/Time.h"
#include "utils/UtlMemory.h"
#include <string.h>
#include <math.h>
#include <new>

CInputPS3::CInputPS3() :
	m_Deadzone(0.1f),
	m_KeyEventCount(0)
{
	memset(m_KeyState, 0, sizeof(m_KeyState));
	memset(m_LastKeyState, 0, sizeof(m_LastKeyState));
	memset(&m_PadInfo, 0, sizeof(padInfo));
	memset(&m_PadData, 0, sizeof(padData));
}

CInputPS3::~CInputPS3()
{
	Shutdown();
}

int32 CInputPS3::Init()
{
	return 1;
}

void CInputPS3::Shutdown()
{
}

void CInputPS3::Update()
{
	memcpy(m_LastKeyState, m_KeyState, sizeof(m_KeyState));
	m_KeyEventCount = 0;

	ioPadGetInfo(&m_PadInfo);
	if (m_PadInfo.status[0])
	{
		ioPadGetData(0, &m_PadData);

		// D-pad and face buttons
		AddKeyEvent('W', m_PadData.BTN_UP || m_PadData.BTN_TRIANGLE);
		AddKeyEvent('A', m_PadData.BTN_LEFT || m_PadData.BTN_SQUARE);
		AddKeyEvent('S', m_PadData.BTN_DOWN || m_PadData.BTN_CROSS);
		AddKeyEvent('D', m_PadData.BTN_RIGHT || m_PadData.BTN_CIRCLE);

		// Start and select
		AddKeyEvent(257, m_PadData.BTN_START);
		AddKeyEvent(256, m_PadData.BTN_SELECT);
		AddKeyEvent('Q', m_PadData.BTN_L1);
		AddKeyEvent('E', m_PadData.BTN_R1);
	}
	else
	{
		AddKeyEvent('W', false);
		AddKeyEvent('A', false);
		AddKeyEvent('S', false);
		AddKeyEvent('D', false);
		AddKeyEvent(257, false);
		AddKeyEvent(256, false);
		AddKeyEvent('Q', false);
		AddKeyEvent('E', false);
	}
}

void CInputPS3::SetWindow(void* pWindow)
{
}

void CInputPS3::SetMouseCaptured(bool isCaptured)
{
}

bool CInputPS3::GetMouseCaptured()
{
	return false;
}

float32 CInputPS3::GetLookX()
{
	if (m_PadInfo.status[0])
	{
		// Right stick X axis
		float32 stickValue = float32(m_PadData.ANA_R_H - 128) / float32(128);
		if (fabsf(stickValue) < m_Deadzone)
		{
			return 0.0f;
		}

		return stickValue;
	}

	return 0.0f;
}

float32 CInputPS3::GetLookY()
{
	if (m_PadInfo.status[0])
	{
		// Right stick Y axis
		float32 stickValue = float32(m_PadData.ANA_R_V - 128) / float32(128);
		if (fabsf(stickValue) < m_Deadzone)
		{
			return 0.0f;
		}

		return stickValue;
	}

	return 0.0f;
}

float32 CInputPS3::GetMoveX()
{
	if (m_PadInfo.status[0])
	{
		// Left stick X axis
		float32 stickValue = float32(m_PadData.ANA_L_H - 128) / float32(128);
		if (fabsf(stickValue) < m_Deadzone)
		{
			return 0.0f;
		}

		return stickValue;
	}

	return 0.0f;
}

float32 CInputPS3::GetMoveY()
{
	if (m_PadInfo.status[0])
	{
		// Left stick Y axis
		float32 stickValue = float32(m_PadData.ANA_L_V - 128) / float32(128);
		if (fabsf(stickValue) < m_Deadzone)
		{
			return 0.0f;
		}

		return -stickValue;
	}

	return 0.0f;
}

bool CInputPS3::IsKeyDown(int32 key)
{
	if (key < 0 || key >= s_MaxKeys)
	{
		return false;
	}

	return m_KeyState[key];
}

bool CInputPS3::WasKeyPressed(int32 key)
{
	if (key < 0 || key >= s_MaxKeys)
	{
		return false;
	}

	return m_KeyState[key] && !m_LastKeyState[key];
}

bool CInputPS3::WasKeyReleased(int32 key)
{
	if (key < 0 || key >= s_MaxKeys)
	{
		return false;
	}

	return !m_KeyState[key] && m_LastKeyState[key];
}

int32 CInputPS3::GetKeyEventCount()
{
	return m_KeyEventCount;
}

int32 CInputPS3::GetKeyEventKey(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return 0;
	}

	return m_KeyEvents[index].m_Key;
}

int32 CInputPS3::GetKeyEventMods(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return 0;
	}

	return m_KeyEvents[index].m_Mods;
}

bool CInputPS3::WasKeyEventPressed(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return false;
	}

	return m_KeyEvents[index].m_IsPress;
}

bool CInputPS3::WasKeyEventReleased(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return false;
	}

	return !m_KeyEvents[index].m_IsPress;
}

float64 CInputPS3::GetKeyEventTime(int32 index)
{
	if (index < 0 || index >= m_KeyEventCount)
	{
		return 0.0;
	}

	return m_KeyEvents[index].m_Time;
}

void CInputPS3::AddKeyEvent(int32 key, bool isDown)
{
	if (isDown == m_KeyState[key] || m_KeyEventCount >= s_MaxKeyEvents)
	{
		return;
	}

	m_KeyState[key] = isDown;

	KeyEvent_t& event = m_KeyEvents[m_KeyEventCount];
	m_KeyEventCount++;

	event.m_Time = CTime::GetTimeSinceStartup();
	event.m_Key = key;
	event.m_Mods = 0;
	event.m_IsPress = isDown;
}

IInput* CreateInput()
{
	void* pMemory = CUtlMemory::Alloc(sizeof(CInputPS3));
	if (pMemory)
	{
		return new(pMemory) CInputPS3();
	}

	return CF_NULL;
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "InputPS3.h"
#include "utils/UtlMemory.h"
#include <string.h>
#include <math.h>
#include <new>

CInputPS3::CInputPS3() :
	m_Deadzone(0.1f)
{
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
	ioPadGetInfo(&m_PadInfo);
	if (m_PadInfo.status[0])
	{
		ioPadGetData(0, &m_PadData);
	}
}

void CInputPS3::SetWindow(void* pWindow)
{
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

IInput* CreateInput()
{
	void* pMemory = CUtlMemory::Alloc(sizeof(CInputPS3));
	if (pMemory)
	{
		return new(pMemory) CInputPS3();
	}

	return CF_NULL;
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "InputManager.h"
#include "Input.h"
#include "dbg.h"
#include "utils/UtlMemory.h"

CInputManager::CInputManager() :
	m_pInput(CF_NULL)
{
}

CInputManager::~CInputManager()
{
	Shutdown();
}

int32 CInputManager::Start()
{
	Shutdown();

	m_pInput = CreateInput();
	if (!m_pInput)
	{
		Error("[CInputManager] Failed to create input\n");

		return 0;
	}

	if (!m_pInput->Init())
	{
		Error("[CInputManager] Failed to initialize input\n");

		m_pInput->~IInput();
		CUtlMemory::Free(m_pInput);
		m_pInput = CF_NULL;

		return 0;
	}

	return 1;
}

void CInputManager::Shutdown()
{
	if (m_pInput)
	{
		m_pInput->Shutdown();
		m_pInput->~IInput();
		CUtlMemory::Free(m_pInput);
		m_pInput = CF_NULL;
	}
}

IInput* CInputManager::Get() const
{
	return m_pInput;
}
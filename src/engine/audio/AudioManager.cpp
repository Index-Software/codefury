//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "AudioManager.h"
#include "Audio.h"
#include "dbg.h"
#include "utils/UtlMemory.h"

CAudioManager::CAudioManager() :
	m_pAudio(CF_NULL)
{
}

CAudioManager::~CAudioManager()
{
	Shutdown();
}

int32 CAudioManager::Start()
{
	Shutdown();

	m_pAudio = CreateAudio();
	if (!m_pAudio)
	{
		Error("[CAudioManager] Failed to create audio\n");

		return 0;
	}

	if (!m_pAudio->Init())
	{
		Error("[CAudioManager] Failed to initialize audio\n");

		m_pAudio->~IAudio();
		CUtlMemory::Free(m_pAudio);
		m_pAudio = CF_NULL;

		return 0;
	}

	return 1;
}

void CAudioManager::Shutdown()
{
	if (m_pAudio)
	{
		m_pAudio->Shutdown();
		m_pAudio->~IAudio();
		CUtlMemory::Free(m_pAudio);
		m_pAudio = CF_NULL;
	}
}

IAudio* CAudioManager::Get() const
{
	return m_pAudio;
}
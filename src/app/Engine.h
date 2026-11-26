//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENGINE_H
#define ENGINE_H

#pragma once

#include "utils/Owned.h"

class CApp;
class CAudioManager;
class CDebug;
class CEntityRegistry;
class CInputManager;
class IRenderer;
class CRenderQueueManager;
class CScene;
class CUI;
class CWrenManager;
class CWindowManager;

class CEngine
{
public:
	CEngine();
	~CEngine();

	bool Init();
	int Run();
private:
	CEngine(const CEngine&);
	CEngine& operator=(const CEngine&);

	COwned<CWindowManager> m_pWindowManager;
	COwned<CInputManager> m_pInputManager;
	COwned<CAudioManager> m_pAudioManager;
	COwned<CUI> m_pGUI;
	COwned<CDebug> m_pDebug;
	COwned<CEntityRegistry> m_pRegistry;
	COwned<CScene> m_pScene;
	COwned<CRenderQueueManager> m_pRenderQueue;
	COwned<CWrenManager> m_pWrenManager;
	IRenderer* m_pRenderer;
	CApp* m_pApp;
};

#endif // ENGINE_H
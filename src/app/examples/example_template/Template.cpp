//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "app/App.h"
#include "window/Window.h"
#include "utils/UtlMemory.h"
#include <new>

class CTemplateApp : public CApp
{
public:
	void GetWindowConfig(WindowConfig_t& windowConfig)
	{
		windowConfig.m_Width = 1600;
		windowConfig.m_Height = 900;
		windowConfig.m_Title = "Template";
	}

	bool Init(IRenderer*, const WindowConfig_t&)
	{
		return true;
	}

	bool Update(IRenderer*, float32)
	{
		return true;
	}
};

CApp* CreateApp()
{
	void* pAppMemory = CUtlMemory::Alloc(sizeof(CTemplateApp));
	if (pAppMemory)
	{
		return new(pAppMemory) CTemplateApp();
	}

	return CF_NULL;
}

void DestroyApp(CApp* pApp)
{
	if (pApp)
	{
		pApp->~CApp();
		CUtlMemory::Free(pApp);
	}
}
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

class CDemoApp : public CApp
{
public:
	void GetWindowConfig(WindowConfig_t& windowConfig)
	{
		windowConfig.m_Title = "Demo";
	}

	const char* GetScriptPath() const
	{
		return "scripts/examples/example_demo/demo.wren";
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
	void* pAppMemory = CUtlMemory::Alloc(sizeof(CDemoApp));
	if (pAppMemory)
	{
		return new(pAppMemory) CDemoApp();
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
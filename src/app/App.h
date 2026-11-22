//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef APP_H
#define APP_H

#pragma once

#include "platform.h"

class IRenderer;
struct WindowConfig_t;

class CApp
{
public:
	CApp()
	{
	}

	virtual ~CApp()
	{
	}

	virtual void GetWindowConfig(WindowConfig_t& windowConfig)
	{
	}

	virtual const char* GetScriptPath() const
	{
		return CF_NULL;
	}

	virtual bool Init(IRenderer* pRenderer,
		const WindowConfig_t& windowConfig) = 0;

	virtual bool Update(IRenderer* pRenderer, float32 deltaTime) = 0;

	virtual void Shutdown()
	{
	}

private:
	CApp(const CApp&);
	CApp& operator=(const CApp&);
};

CApp* CreateApp();
void DestroyApp(CApp* pApp);

#endif // APP_H
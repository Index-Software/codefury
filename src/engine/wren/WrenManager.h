//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef WREN_MANAGER_H
#define WREN_MANAGER_H

#pragma once

#include "platform.h"
#include "wren.hpp"

class CWrenManager
{
public:
	CWrenManager();
	~CWrenManager();

	bool Init();
	void Shutdown();

	WrenVM* GetVM() const;
	WrenConfiguration& GetConfig();

	bool LoadScript(const char* pScriptPath);

	bool Update(float32 deltaTime);

	static WrenLoadModuleResult LoadScriptModule(
		WrenVM* pVM,
		const char* pName);
private:
	CWrenManager(const CWrenManager&);
	CWrenManager& operator=(const CWrenManager&);

	void ReleaseHandles();

	WrenConfiguration m_Config;
	WrenVM* m_pVM;

	WrenHandle* m_pClass;
	WrenHandle* m_pInstance;
	WrenHandle* m_pUpdateHandle;
};

#endif // WREN_MANAGER_H
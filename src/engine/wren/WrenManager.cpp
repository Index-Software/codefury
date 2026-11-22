//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "engine/wren/WrenManager.h"
#include "WrenHandle.h"
#include "Runtime.h"
#include "Class.h"
#include "WrenBind.h"
#include "renderer/Renderer.h"
#include "utils/Utils.h"
#include "dbg.h"
#include <stdlib.h>
#include <string.h>

static void FreeModuleSource(
	WrenVM* pVM,
	const char* pName,
	WrenLoadModuleResult result)
{
	free(const_cast<char*>(result.source));
}

class CUI;
class CDebug;
class CEntityRegistry;
class CScene;
class CRenderQueueManager;
class IInput;

static bool CallMethod(
	WrenVM* pVM,
	WrenHandle* pReceiver,
	const char* pSignature)
{
	WrenHandle* pCall = wrenMakeCallHandle(pVM, pSignature);
	wrenEnsureSlots(pVM, 1);
	wrenSetSlotHandle(pVM, 0, pReceiver);
	bool success = (wrenCall(pVM, pCall) == WREN_RESULT_SUCCESS);
	wrenReleaseHandle(pVM, pCall);

	return success;
}

CWrenManager::CWrenManager() :
	m_pVM(CF_NULL),
	m_pClass(CF_NULL),
	m_pInstance(CF_NULL),
	m_pUpdateHandle(CF_NULL)
{
	wrenInitConfiguration(&m_Config);

	m_Config.loadModuleFn = LoadScriptModule;
	WrenSetupConfig(&m_Config);
}

CWrenManager::~CWrenManager()
{
	Shutdown();
}

bool CWrenManager::Init()
{
	if (m_pVM)
	{
		return false;
	}

	m_pVM = wrenNewVM(&m_Config);
	if (!CWrenBind::Init(m_pVM))
	{
		wrenFreeVM(m_pVM);
		m_pVM = CF_NULL;

		return false;
	}

	return (m_pVM != CF_NULL);
}

void CWrenManager::Shutdown()
{
	ReleaseHandles();

	if (m_pVM)
	{
		wrenFreeVM(m_pVM);
		m_pVM = CF_NULL;
	}

	CWrenBindSingleton<IRenderer>::Set(WB_NULL);
	CWrenBindSingleton<CUI>::Set(WB_NULL);
	CWrenBindSingleton<CDebug>::Set(WB_NULL);
	CWrenBindSingleton<CEntityRegistry>::Set(WB_NULL);
	CWrenBindSingleton<CScene>::Set(WB_NULL);
	CWrenBindSingleton<CRenderQueueManager>::Set(WB_NULL);
	CWrenBindSingleton<IInput>::Set(WB_NULL);
}

WrenVM* CWrenManager::GetVM() const
{
	return m_pVM;
}

WrenConfiguration& CWrenManager::GetConfig()
{
	return m_Config;
}

bool CWrenManager::LoadScript(const char* pScriptPath)
{
	WrenVM* pVM = m_pVM;

	CFixedString scriptSource = CUtils::ReadFile(CFixedString(pScriptPath));
	if (scriptSource.Length() == 0)
	{
		Error("[CWrenManager] Failed to load script %s\n", pScriptPath);

		return false;
	}

	if (wrenInterpret(pVM, "main", scriptSource.c_str()) != WREN_RESULT_SUCCESS)
	{
		Error("[CWrenManager] Failed to compile script\n");

		return false;
	}

	wrenEnsureSlots(pVM, 1);
	wrenGetVariable(pVM, "main", "App", 0);
	CWrenHandle hClass(pVM, wrenGetSlotHandle(pVM, 0));

	if (!CallMethod(pVM, hClass, "new()"))
	{
		Error("[CWrenManager] Failed to construct\n");

		return false;
	}

	CWrenHandle hInstance(pVM, wrenGetSlotHandle(pVM, 0));

	if (!CallMethod(pVM, hInstance, "init()"))
	{
		Error("[CWrenManager] Failed to init\n");

		return false;
	}

	m_pClass = hClass.Detach();
	m_pInstance = hInstance.Detach();
	m_pUpdateHandle = wrenMakeCallHandle(pVM, "update(_)");

	return true;
}

bool CWrenManager::Update(float32 deltaTime)
{
	WrenVM* pVM = m_pVM;

	wrenEnsureSlots(pVM, 2);
	wrenSetSlotHandle(pVM, 0, m_pInstance);
	wrenSetSlotDouble(pVM, 1, static_cast<double>(deltaTime));

	if (wrenCall(pVM, m_pUpdateHandle) != WREN_RESULT_SUCCESS)
	{
		Error("[CWrenManager] Failed to update\n");

		return false;
	}

	return true;
}

WrenLoadModuleResult CWrenManager::LoadScriptModule(
	WrenVM* pVM,
	const char* pName)
{
	WrenLoadModuleResult result = { CF_NULL, CF_NULL, CF_NULL };

	CFixedString path = CFixedString("scripts/") + pName +
		CFixedString(".wren");
	CFixedString contents = CUtils::ReadFile(path);
	if (contents.Length() == 0)
	{
		Error("[CWrenManager] Failed to open module: %s\n", pName);

		return result;
	}

	char* source = static_cast<char*>(malloc(contents.Length() + 1));
	if (!source)
	{
		return result;
	}

	memcpy(source, contents.c_str(), contents.Length() + 1);
	result.source = source;
	result.onComplete = FreeModuleSource;

	return result;
}

void CWrenManager::ReleaseHandles()
{
	if (m_pUpdateHandle)
	{
		wrenReleaseHandle(m_pVM, m_pUpdateHandle);
		m_pUpdateHandle = CF_NULL;
	}

	if (m_pInstance)
	{
		wrenReleaseHandle(m_pVM, m_pInstance);
		m_pInstance = CF_NULL;
	}

	if (m_pClass)
	{
		wrenReleaseHandle(m_pVM, m_pClass);
		m_pClass = CF_NULL;
	}
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

extern "C" {
#include "wren.h"
}
#include "Handle.h"
#include "WrenBind.h"
#include "wren/wrenbind/WrenBindArg.h"
#include "ecs/EntityRegistry.h"
#include "ecs/components/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/RenderQueue.h"
#include "Maths.h"
#include "wren/wrenbind/generated/WrenBindGenerated.h"

void WrenFwd_CEntityRegistry_AddComponent(WrenVM* pVM)
{
	uint32 id = static_cast<uint32>(wrenGetSlotDouble(pVM, 1));
	if (wrenGetSlotType(pVM, 2) == WREN_TYPE_FOREIGN)
	{
		void* pForeignSlot = wrenGetSlotForeign(pVM, 2);
		Handle_t* pComponentHandle = static_cast<Handle_t*>(pForeignSlot);
		CEntityComponent* pComponent = static_cast<CEntityComponent*>(
			HandleToPtr(*pComponentHandle));
		CWrenBindSingleton<CEntityRegistry>::s_pInstance->AddComponent(
			id,
			pComponent);
	}
	else
	{
		CWrenBindSingleton<CEntityRegistry>::s_pInstance->AddComponent(
			id,
			static_cast<CEntityComponent*>(HandleToPtr(
				static_cast<Handle_t>(wrenGetSlotDouble(pVM, 2)))));
	}
}

void WrenFwd_CEntityRegistry_CreateEntity2(WrenVM* pVM)
{
	wrenSetSlotDouble(
		pVM,
		0,
		static_cast<double>(
			CWrenBindSingleton<CEntityRegistry>::s_pInstance->CreateEntity(
				wrenGetSlotString(pVM, 1),
				CWrenBindArg<CVector3>::Get(pVM, 2)).GetID()));
}

void WrenFwd_CEntityRegistry_CreateEntity3(WrenVM* pVM)
{
	wrenSetSlotDouble(
		pVM,
		0,
		static_cast<double>(
			CWrenBindSingleton<CEntityRegistry>::s_pInstance->CreateEntity(
				wrenGetSlotString(pVM, 1),
				CWrenBindArg<CVector3>::Get(pVM, 2),
				CWrenBindArg<CVector3>::Get(pVM, 3)).GetID()));
}

void WrenFwd_CRenderQueueManager_Render(WrenVM* pVM)
{
	CCamera* pCamera = static_cast<CCamera*>(HandleToPtr(
		*static_cast<Handle_t*>(wrenGetSlotForeign(pVM, 1))));
	CWrenBindSingleton<CRenderQueueManager>::s_pInstance->Render(
		*CWrenBindSingleton<IRenderer>::s_pInstance,
		pCamera->GetViewProjectionMatrix(),
		pCamera);
}

void WrenFwd_CMaths_DirectionToEquirect(WrenVM* pVM)
{
	EquirectUV_t equirectUV = CMaths::DirectionToEquirect(
		CWrenBindArg<CVector3>::Get(pVM, 1));
	CWrenBindArg<CVector2>::Set(pVM, CVector2(equirectUV.m_U, equirectUV.m_V));
}
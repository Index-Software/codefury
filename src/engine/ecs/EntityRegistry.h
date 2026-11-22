//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENTITY_REGISTRY_H
#define ENTITY_REGISTRY_H

#pragma once

#include "Entity.h"
#include "EntityComponent.h"
#include "utils/RefCounted.h"
#include "utils/UtlVector.h"
#include "utils/UtlMap.h"
#include "utils/FixedString.h"
#include "mathsfury/Maths.h"
#include "WrenBind.h"

class CCamera;
class CTransform;

class CEntityRegistry : public CWrenBindSingleton<CEntityRegistry>
{
	WREN_SINGLETON_DECL()
public:
	CEntityRegistry();

	CEntity CreateEntity() WREN_BIND_SINGLETON("createEntity()", s_pRegistry);
	CEntity CreateEntity(const char* pName)
		WREN_BIND_SINGLETON("createEntity(_)", s_pRegistry);
	CEntity CreateEntity(
		const char* pName,
		const CVector3& position,
		const CVector3& rotation = CVector3(0.0f, 0.0f, 0.0f),
		const CVector3& scale = CVector3(1.0f, 1.0f, 1.0f))
		WREN_BIND_FWD(
			"createEntity(_,_)", "WrenFwd_CEntityRegistry_CreateEntity2")
		WREN_BIND_FWD(
			"createEntity(_,_,_)", "WrenFwd_CEntityRegistry_CreateEntity3")
		WREN_BIND_SINGLETON("createEntity(_,_,_,_)", s_pRegistry);
	CUtlVector<CEntity> FindEntities(const char* pComponentTypeName)
		WREN_BIND_SINGLETON("findEntities(_)", s_pRegistry);
	CEntity FindEntityByName(const char* pName)
		WREN_BIND_SINGLETON("findEntityByName(_)", s_pRegistry);
	void SetEntityName(uint32 hID, const char* pName)
		WREN_BIND_SINGLETON("setEntityName(_,_)", s_pRegistry);
	const char* GetEntityName(uint32 hID) const
		WREN_BIND_SINGLETON("getEntityName(_)", s_pRegistry);
	void DestroyEntity(uint32 hID)
		WREN_BIND_SINGLETON("destroyEntity(_)", s_pRegistry);

	void AddComponent(uint32 hID, CEntityComponent* pComp)
		WREN_BIND_FWD(
			"addComponent(_,_)", "WrenFwd_CEntityRegistry_AddComponent");
	template<typename T>
	T* AddComponent(uint32 hID)
	{
		CRef<T> component = CRef<T>::Create();
		if (!component.IsValid())
		{
			return CF_NULL;
		}

		AddComponent(hID, component.Get());

		return component.Get();
	}

	void RemoveComponents(uint32 hID)
		WREN_BIND_SINGLETON("removeComponents(_)", s_pRegistry);
	const CUtlVector<CEntityComponent*>* GetComponents(uint32 hID) const;

	CRef<CEntityComponent> GetComponentByType(uint32 hID, const char* pTypeName)
		WREN_BIND_SINGLETON("getComponent(_,_)", s_pRegistry)
		WREN_BIND_SINGLETON("getComponentByType(_,_)", s_pRegistry);
	const CEntityComponent* GetComponentByType(
		uint32 hID,
		const char* pTypeName) const;

	template<typename T>
	T* GetComponent(uint32 hID)
	{
		return static_cast<T*>(
			GetComponentByType(hID, T().TypeName()).Get());
	}

	template<typename T>
	const T* GetComponent(uint32 hID) const
	{
		return static_cast<const T*>(GetComponentByType(hID, T().TypeName()));
	}

	void SetParent(uint32 hID, uint32 hParentID)
		WREN_BIND_SINGLETON("setParent(_,_)", s_pRegistry);
	uint32 GetParent(uint32 hID) const
		WREN_BIND_SINGLETON("getParent(_)", s_pRegistry);
	const CUtlVector<uint32>* GetChildren(uint32 hID) const
		WREN_BIND_SINGLETON("getChildren(_)", s_pRegistry);
	CCamera* GetCameraComponent(uint32 hID);
	CTransform* GetTransformComponent(uint32 hID);
private:
	CUtlMap<uint32, CFixedString> m_Names;
	CUtlMap<uint32, CRefList<CEntityComponent> > m_Components;
	CUtlMap<uint32, uint32> m_Parents;
	CUtlMap<uint32, CUtlVector<uint32> > m_Children;

	uint32 m_NextID;
};

#endif // ENTITY_REGISTRY_H
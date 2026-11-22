//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "EntityRegistry.h"
#include "Entity.h"
#include "components/Camera.h"
#include "components/Transform.h"
#include "platform.h"
#include "dbg.h"
#include <string.h>

CEntityRegistry::CEntityRegistry() :
	m_NextID(1)
{
}

CEntity CEntityRegistry::CreateEntity()
{
	uint32 hID = m_NextID++;
	m_Names.Insert(hID, CFixedString(""));
	m_Components.Insert(hID, CRefList<CEntityComponent>());
	m_Children.Insert(hID, CUtlVector<uint32>());
	m_Parents.Insert(hID, 0);

	return CEntity(hID, this);
}

CEntity CEntityRegistry::CreateEntity(const char* pName)
{
	CEntity entity = CreateEntity();
	SetEntityName(entity.GetID(), pName);

	AddComponent<CTransform>(entity.GetID());

	return entity;
}

CEntity CEntityRegistry::CreateEntity(
	const char* pName,
	const CVector3& position,
	const CVector3& rotation,
	const CVector3& scale)
{
	CEntity entity = CreateEntity(pName);
	CTransform* pTransform = GetTransformComponent(entity.GetID());
	if (pTransform)
	{
		pTransform->m_Position = position;
		pTransform->m_Rotation = rotation;
		pTransform->m_Scale = scale;
	}

	return entity;
}

CUtlVector<CEntity> CEntityRegistry::FindEntities(
	const char* pComponentTypeName)
{
	CUtlVector<CEntity> result;
	for (int32 i = 0; i < m_Components.Count(); i++)
	{
		uint32 hID = m_Components.Key(i);
		const CUtlVector<CEntityComponent*>& components =
			m_Components.Element(i).Items();
		for (int32 j = 0; j < components.Count(); j++)
		{
			if (components[j]->TypeName() &&
				strcmp(components[j]->TypeName(), pComponentTypeName) == 0)
			{
				result.AddToTail(CEntity(hID, this));

				break;
			}
		}
	}

	return result;
}

CEntity CEntityRegistry::FindEntityByName(const char* pName)
{
	for (int32 i = 0; i < m_Names.Count(); i++)
	{
		if (m_Names.Element(i) == pName)
		{
			return CEntity(m_Names.Key(i), this);
		}
	}

	return CEntity();
}

void CEntityRegistry::SetEntityName(uint32 hID, const char* pName)
{
	int32 index = m_Names.Find(hID);
	if (index != m_Names.InvalidIndex())
	{
		m_Names.Element(index) = pName;
	}
}

const char* CEntityRegistry::GetEntityName(uint32 hID) const
{
	int32 index = m_Names.Find(hID);
	if (index != m_Names.InvalidIndex())
	{
		return m_Names.Element(index).c_str();
	}

	return CF_NULL;
}

void CEntityRegistry::DestroyEntity(uint32 hID)
{
	CUtlVector<uint32> doomed;
	doomed.AddToTail(hID);
	for (int32 i = 0; i < doomed.Count(); i++)
	{
		int32 childIndex = m_Children.Find(doomed[i]);
		if (childIndex == m_Children.InvalidIndex())
		{
			continue;
		}

		const CUtlVector<uint32>& children = m_Children.Element(childIndex);
		for (int32 j = 0; j < children.Count(); j++)
		{
			if (doomed.Find(children[j]) == -1)
			{
				doomed.AddToTail(children[j]);
			}
		}
	}

	int32 parentIndex = m_Parents.Find(hID);
	if (parentIndex != m_Parents.InvalidIndex() &&
		m_Parents.Element(parentIndex) != 0)
	{
		uint32 hParentID = m_Parents.Element(parentIndex);
		int32 parentChildrenIndex = m_Children.Find(hParentID);
		if (parentChildrenIndex != m_Children.InvalidIndex())
		{
			m_Children.Element(parentChildrenIndex).FindAndRemove(hID);
		}
	}

	for (int32 i = doomed.Count() - 1; i >= 0; i--)
	{
		uint32 hDoomedID = doomed[i];

		RemoveComponents(hDoomedID);

		m_Components.Remove(hDoomedID);
		m_Children.Remove(hDoomedID);
		m_Parents.Remove(hDoomedID);
		m_Names.Remove(hDoomedID);
	}
}

void CEntityRegistry::AddComponent(uint32 hID, CEntityComponent* pComp)
{
	if (pComp)
	{
		int32 index = m_Components.Find(hID);
		if (index == m_Components.InvalidIndex())
		{
			m_Components.Insert(hID, CRefList<CEntityComponent>());
			index = m_Components.Find(hID);
		}

		m_Components.Element(index).Add(pComp);
	}
}

void CEntityRegistry::RemoveComponents(uint32 hID)
{
	int32 index = m_Components.Find(hID);
	if (index != m_Components.InvalidIndex())
	{
		m_Components.Element(index).RemoveAll();
	}
}

const CUtlVector<CEntityComponent*>* CEntityRegistry::GetComponents(
	uint32 hID) const
{
	int32 index = m_Components.Find(hID);

	return (index != m_Components.InvalidIndex())
		? &m_Components.Element(index).Items()
		: CF_NULL;
}

CRef<CEntityComponent> CEntityRegistry::GetComponentByType(
	uint32 hID,
	const char* pTypeName)
{
	int32 index = m_Components.Find(hID);
	if (index == m_Components.InvalidIndex())
	{
		return CRef<CEntityComponent>();
	}

	const CUtlVector<CEntityComponent*>& components =
		m_Components[index].Items();
	for (int32 i = 0; i < components.Count(); i++)
	{
		if (components[i]->IsType(pTypeName))
		{
			return CRef<CEntityComponent>(components[i]);
		}
	}

	return CRef<CEntityComponent>();
}

const CEntityComponent* CEntityRegistry::GetComponentByType(
	uint32 hID,
	const char* pTypeName) const
{
	int32 index = m_Components.Find(hID);
	if (index == m_Components.InvalidIndex())
	{
		return CF_NULL;
	}

	const CUtlVector<CEntityComponent*>& components =
		m_Components.Element(index).Items();
	for (int32 i = 0; i < components.Count(); i++)
	{
		if (components[i]->IsType(pTypeName))
		{
			return components[i];
		}
	}

	return CF_NULL;
}

void CEntityRegistry::SetParent(uint32 hID, uint32 hParentID)
{
	uint32 hAncestorID = hParentID;
	for (int32 step = 0;
		hAncestorID != 0 && step <= m_Parents.Count();
		step++)
	{
		if (hAncestorID == hID)
		{
			Error("[CEntityRegistry] SetParent would create a cycle\n");

			return;
		}

		hAncestorID = GetParent(hAncestorID);
	}

	int32 parentIndex = m_Parents.Find(hID);
	if (parentIndex != m_Parents.InvalidIndex() &&
		m_Parents.Element(parentIndex) != 0)
	{
		uint32 hOldParentID = m_Parents.Element(parentIndex);
		int32 oldParentChildrenIndex = m_Children.Find(hOldParentID);
		if (oldParentChildrenIndex != m_Children.InvalidIndex())
		{
			m_Children.Element(oldParentChildrenIndex).FindAndRemove(hID);
		}
	}

	if (parentIndex != m_Parents.InvalidIndex())
	{
		m_Parents.Element(parentIndex) = hParentID;
	}
	else
	{
		m_Parents.Insert(hID, hParentID);
	}

	if (hParentID != 0)
	{
		int32 childrenIndex = m_Children.Find(hParentID);
		if (childrenIndex == m_Children.InvalidIndex())
		{
			m_Children.Insert(hParentID, CUtlVector<uint32>());
			childrenIndex = m_Children.Find(hParentID);
		}
		m_Children.Element(childrenIndex).AddToTail(hID);
	}
}

uint32 CEntityRegistry::GetParent(uint32 hID) const
{
	int32 index = m_Parents.Find(hID);

	return (index != m_Parents.InvalidIndex()) ? m_Parents.Element(index) : 0;
}

const CUtlVector<uint32>* CEntityRegistry::GetChildren(uint32 hID) const
{
	int32 index = m_Children.Find(hID);

	return (index != m_Children.InvalidIndex())
		? &m_Children.Element(index)
		: CF_NULL;
}

CCamera* CEntityRegistry::GetCameraComponent(uint32 hID)
{
	int32 index = m_Components.Find(hID);
	if (index == m_Components.InvalidIndex())
	{
		return CF_NULL;
	}

	const CUtlVector<CEntityComponent*>& components =
		m_Components.Element(index).Items();
	for (int32 i = 0; i < components.Count(); i++)
	{
		CEntityComponent* pComp = components[i];
		if (!pComp)
		{
			continue;
		}

		const char* pName = pComp->TypeName();
		if (!pName)
		{
			continue;
		}

		if (strcmp(pName, "CCamera") == 0)
		{
			return static_cast<CCamera*>(pComp);
		}
	}

	return CF_NULL;
}

CTransform* CEntityRegistry::GetTransformComponent(uint32 hID)
{
	int32 index = m_Components.Find(hID);
	if (index == m_Components.InvalidIndex())
	{
		return CF_NULL;
	}

	const CUtlVector<CEntityComponent*>& components =
		m_Components.Element(index).Items();
	for (int32 i = 0; i < components.Count(); i++)
	{
		CEntityComponent* pComp = components[i];
		if (pComp && pComp->TypeName() &&
			strcmp(pComp->TypeName(), "CTransform") == 0)
		{
			return static_cast<CTransform*>(pComp);
		}
	}

	return CF_NULL;
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENTITY_H
#define ENTITY_H

#pragma once

#include "platform.h"

class CEntityRegistry;

class CEntity
{
public:
	CEntity() :
		m_pRegistry(CF_NULL),
		m_hID(0)
	{
	}

	CEntity(uint32 hID, CEntityRegistry* pRegistry) :
		m_pRegistry(pRegistry),
		m_hID(hID)
	{
	}

	void SetName(const char* pName);
	const char* GetName() const;

	uint32 GetID() const
	{
		return m_hID;
	}

	bool IsValid() const
	{
		return m_hID != 0 && m_pRegistry != CF_NULL;
	}

	CEntityRegistry* GetRegistry() const
	{
		return m_pRegistry;
	}
private:
	CEntityRegistry* m_pRegistry;
	uint32 m_hID;
};

#endif // ENTITY_H
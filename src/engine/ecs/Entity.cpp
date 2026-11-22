//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Entity.h"
#include "EntityRegistry.h"

void CEntity::SetName(const char* pName)
{
	if (m_pRegistry)
	{
		m_pRegistry->SetEntityName(m_hID, pName);
	}
}

const char* CEntity::GetName() const
{
	return m_pRegistry ? m_pRegistry->GetEntityName(m_hID) : CF_NULL;
}
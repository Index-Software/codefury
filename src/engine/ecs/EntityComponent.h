//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENTITY_COMPONENT_H
#define ENTITY_COMPONENT_H

#pragma once

#include "platform.h"
#include "utils/RefCounted.h"
#include <string.h>

class CEntityComponent : public CRefCounted
{
public:
	virtual ~CEntityComponent() CF_OVERRIDE
	{
	}

	virtual const char* TypeName() const
	{
		return "EntityComponent";
	}

	virtual bool IsType(const char* pTypeName) const
	{
		return strcmp(TypeName(), pTypeName) == 0;
	}
};

#endif // ENTITY_COMPONENT_H
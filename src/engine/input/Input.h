//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef INPUT_H
#define INPUT_H

#pragma once

#include "platform.h"
#include "WrenBind.h"

class IInput : public CWrenBindSingleton<IInput>
{
	WREN_SINGLETON_DECL()
public:
	virtual ~IInput()
	{
	}

	virtual int32 Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Update() = 0;
	virtual void SetWindow(void* pWindow) = 0;

	virtual float32 GetLookX() WREN_BIND_SINGLETON("getLookX()", s_pInput) = 0;
	virtual float32 GetLookY() WREN_BIND_SINGLETON("getLookY()", s_pInput) = 0;
	virtual float32 GetMoveX() WREN_BIND_SINGLETON("getMoveX()", s_pInput) = 0;
	virtual float32 GetMoveY() WREN_BIND_SINGLETON("getMoveY()", s_pInput) = 0;
};

IInput* CreateInput();

#endif // INPUT_H
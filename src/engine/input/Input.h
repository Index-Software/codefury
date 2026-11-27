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

static const int32 s_MaxKeyEvents = 64;
static const int32 s_MaxKeys = 512;

struct KeyEvent_t
{
	float64 m_Time;
	int32 m_Key;
	int32 m_Mods;
	bool m_IsPress;
};

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
	virtual void SetMouseCaptured(bool isCaptured)
		WREN_BIND_SINGLETON("setMouseCaptured(_)", s_pInput) = 0;
	virtual bool GetMouseCaptured()
		WREN_BIND_SINGLETON("getMouseCaptured()", s_pInput) = 0;

	virtual float32 GetLookX() WREN_BIND_SINGLETON("getLookX()", s_pInput) = 0;
	virtual float32 GetLookY() WREN_BIND_SINGLETON("getLookY()", s_pInput) = 0;
	virtual float32 GetMoveX() WREN_BIND_SINGLETON("getMoveX()", s_pInput) = 0;
	virtual float32 GetMoveY() WREN_BIND_SINGLETON("getMoveY()", s_pInput) = 0;

	virtual bool IsKeyDown(int32 key)
		WREN_BIND_SINGLETON("isKeyDown(_)", s_pInput) = 0;
	virtual bool WasKeyPressed(int32 key)
		WREN_BIND_SINGLETON("wasKeyPressed(_)", s_pInput) = 0;
	virtual bool WasKeyReleased(int32 key)
		WREN_BIND_SINGLETON("wasKeyReleased(_)", s_pInput) = 0;
	virtual int32 GetKeyEventCount()
		WREN_BIND_SINGLETON("getKeyEventCount()", s_pInput) = 0;
	virtual int32 GetKeyEventKey(int32 index)
		WREN_BIND_SINGLETON("getKeyEventKey(_)", s_pInput) = 0;
	virtual int32 GetKeyEventMods(int32 index)
		WREN_BIND_SINGLETON("getKeyEventMods(_)", s_pInput) = 0;
	virtual bool WasKeyEventPressed(int32 index)
		WREN_BIND_SINGLETON("wasKeyEventPressed(_)", s_pInput) = 0;
	virtual bool WasKeyEventReleased(int32 index)
		WREN_BIND_SINGLETON("wasKeyEventReleased(_)", s_pInput) = 0;
	virtual float64 GetKeyEventTime(int32 index)
		WREN_BIND_SINGLETON("getKeyEventTime(_)", s_pInput) = 0;
};

IInput* CreateInput();

#endif // INPUT_H
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef INPUT_PS3_H
#define INPUT_PS3_H

#pragma once

#include "../Input.h"
#include <io/pad.h>

class CInputPS3 : public IInput
{
public:
	CInputPS3();
	~CInputPS3() CF_OVERRIDE;

	int32 Init() CF_OVERRIDE;
	void Shutdown() CF_OVERRIDE;
	void Update() CF_OVERRIDE;
	void SetWindow(void* pWindow) CF_OVERRIDE;
	void SetMouseCaptured(bool isCaptured) CF_OVERRIDE;
	bool GetMouseCaptured() CF_OVERRIDE;

	float32 GetLookX() CF_OVERRIDE;
	float32 GetLookY() CF_OVERRIDE;
	float32 GetMoveX() CF_OVERRIDE;
	float32 GetMoveY() CF_OVERRIDE;

	bool IsKeyDown(int32 key) CF_OVERRIDE;
	bool WasKeyPressed(int32 key) CF_OVERRIDE;
	bool WasKeyReleased(int32 key) CF_OVERRIDE;
	int32 GetKeyEventCount() CF_OVERRIDE;
	int32 GetKeyEventKey(int32 index) CF_OVERRIDE;
	int32 GetKeyEventMods(int32 index) CF_OVERRIDE;
	bool WasKeyEventPressed(int32 index) CF_OVERRIDE;
	bool WasKeyEventReleased(int32 index) CF_OVERRIDE;
	float64 GetKeyEventTime(int32 index) CF_OVERRIDE;
private:
	void AddKeyEvent(int32 key, bool isDown);

	KeyEvent_t m_KeyEvents[s_MaxKeyEvents];
	padInfo m_PadInfo;
	padData m_PadData;
	float32 m_Deadzone;
	int32 m_KeyEventCount;
	bool m_KeyState[s_MaxKeys];
	bool m_LastKeyState[s_MaxKeys];
};

#endif // INPUT_PS3_H
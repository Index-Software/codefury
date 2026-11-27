//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef INPUT_GLFW_H
#define INPUT_GLFW_H

#pragma once

#include "../Input.h"

struct GLFWwindow;

class CInputGLFW : public IInput
{
public:
	CInputGLFW();
	~CInputGLFW() CF_OVERRIDE;

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
	static void OnKey(
		GLFWwindow* pWindow,
		int key,
		int scancode,
		int action,
		int mods);

	void SetMouseActive(bool active);

	KeyEvent_t m_KeyEvents[s_MaxKeyEvents];
	KeyEvent_t m_PendingKeyEvents[s_MaxKeyEvents];
	GLFWwindow* m_pWindow;
	float64 m_PreviousMouseX;
	float64 m_PreviousMouseY;
	float32 m_AccumulatedLookX;
	float32 m_AccumulatedLookY;
	int32 m_KeyEventCount;
	int32 m_PendingKeyEventCount;
	bool m_KeyState[s_MaxKeys];
	bool m_LastKeyState[s_MaxKeys];
	bool m_IsMouseActive;
	bool m_HasMouseOrigin;
};

#endif // INPUT_GLFW_H
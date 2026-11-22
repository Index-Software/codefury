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

	float32 GetLookX() CF_OVERRIDE;
	float32 GetLookY() CF_OVERRIDE;
	float32 GetMoveX() CF_OVERRIDE;
	float32 GetMoveY() CF_OVERRIDE;
private:
	void SetMouseActive(bool active);

	GLFWwindow* m_pWindow;
	float64 m_PreviousMouseX;
	float64 m_PreviousMouseY;
	float32 m_AccumulatedLookX;
	float32 m_AccumulatedLookY;
	bool m_IsMouseActive;
	bool m_HasMouseOrigin;
};

#endif // INPUT_GLFW_H
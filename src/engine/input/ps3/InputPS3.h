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

	float32 GetLookX() CF_OVERRIDE;
	float32 GetLookY() CF_OVERRIDE;
	float32 GetMoveX() CF_OVERRIDE;
	float32 GetMoveY() CF_OVERRIDE;
private:
	padInfo m_PadInfo;
	padData m_PadData;
	float32 m_Deadzone;
};

#endif // INPUT_PS3_H
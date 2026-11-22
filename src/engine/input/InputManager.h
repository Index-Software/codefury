//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#pragma once

#include "platform.h"

class IInput;

struct InputBackend_t
{
	enum Enum
	{
		INPUT_GLFW = 0,
		INPUT_PS3 = 1,
		INPUT_BACKEND_COUNT
	};
};

class CInputManager
{
public:
	CInputManager();
	~CInputManager();

	int32 Start();
	void Shutdown();

	IInput* Get() const;
private:
	IInput* m_pInput;
};

#endif // INPUT_MANAGER_H
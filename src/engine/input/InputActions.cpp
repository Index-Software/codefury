//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "input/InputActions.h"
#include "input/Input.h"
#include "utils/UtlVector.h"
#include <string.h>

static const int32 s_MaxBindings = 4;
static const int32 s_MaxNameLength = 32;
static const int32 s_ModsMask = 0xF;

static const int32 s_KeyLeftShift = 340;
static const int32 s_KeyLeftControl = 341;
static const int32 s_KeyLeftAlt = 342;
static const int32 s_KeyLeftSuper = 343;
static const int32 s_KeyRightShift = 344;
static const int32 s_KeyRightControl = 345;
static const int32 s_KeyRightAlt = 346;
static const int32 s_KeyRightSuper = 347;

struct InputBinding_t
{
	int32 m_Key;
	int32 m_Mods;
};

struct InputAction_t
{
	InputBinding_t m_Bindings[s_MaxBindings];
	int32 m_BindingCount;
	char m_Name[s_MaxNameLength];
};

static IInput* s_pActiveInput = CF_NULL;
static CUtlVector<InputAction_t> s_Actions;

static InputAction_t* FindAction(const char* pName)
{
	for (int32 i = 0; i < s_Actions.Count(); i++)
	{
		if (strcmp(s_Actions[i].m_Name, pName) == 0)
		{
			return &s_Actions[i];
		}
	}

	return CF_NULL;
}

static InputAction_t* FindOrAddAction(const char* pName)
{
	InputAction_t* pAction = FindAction(pName);
	if (pAction)
	{
		return pAction;
	}

	InputAction_t action;
	memset(&action, 0, sizeof(action));
	strncpy(action.m_Name, pName, s_MaxNameLength - 1);
	s_Actions.AddToTail(action);

	return &s_Actions[s_Actions.Count() - 1];
}

static int32 GetHeldMods()
{
	int32 mods = InputMods_t::None;
	if (s_pActiveInput->IsKeyDown(s_KeyLeftShift)
		|| s_pActiveInput->IsKeyDown(s_KeyRightShift))
	{
		mods |= InputMods_t::Shift;
	}
	if (s_pActiveInput->IsKeyDown(s_KeyLeftControl)
		|| s_pActiveInput->IsKeyDown(s_KeyRightControl))
	{
		mods |= InputMods_t::Control;
	}
	if (s_pActiveInput->IsKeyDown(s_KeyLeftAlt)
		|| s_pActiveInput->IsKeyDown(s_KeyRightAlt))
	{
		mods |= InputMods_t::Alt;
	}
	if (s_pActiveInput->IsKeyDown(s_KeyLeftSuper)
		|| s_pActiveInput->IsKeyDown(s_KeyRightSuper))
	{
		mods |= InputMods_t::Super;
	}

	return mods;
}

void CInputActions::SetInput(IInput* pInput)
{
	s_pActiveInput = pInput;
}

bool CInputActions::Bind(const char* pName, int32 key, int32 mods)
{
	InputAction_t* pAction = FindOrAddAction(pName);
	mods &= s_ModsMask;
	for (int32 i = 0; i < pAction->m_BindingCount; i++)
	{
		if (pAction->m_Bindings[i].m_Key == key
			&& pAction->m_Bindings[i].m_Mods == mods)
		{
			return true;
		}
	}

	if (pAction->m_BindingCount >= s_MaxBindings)
	{
		return false;
	}

	InputBinding_t& binding = pAction->m_Bindings[pAction->m_BindingCount];
	binding.m_Key = key;
	binding.m_Mods = mods;
	pAction->m_BindingCount++;

	return true;
}

bool CInputActions::SetBinding(
	const char* pName,
	int32 index,
	int32 key,
	int32 mods)
{
	InputAction_t* pAction = FindOrAddAction(pName);
	if (index < 0 || index >= pAction->m_BindingCount)
	{
		return false;
	}

	pAction->m_Bindings[index].m_Key = key;
	pAction->m_Bindings[index].m_Mods = mods & s_ModsMask;

	return true;
}

void CInputActions::ClearBindings(const char* pName)
{
	FindOrAddAction(pName)->m_BindingCount = 0;
}

int32 CInputActions::GetBindingCount(const char* pName)
{
	const InputAction_t* pAction = FindAction(pName);

	return pAction ? pAction->m_BindingCount : 0;
}

int32 CInputActions::GetBindingKey(const char* pName, int32 index)
{
	const InputAction_t* pAction = FindAction(pName);
	if (!pAction || index < 0 || index >= pAction->m_BindingCount)
	{
		return 0;
	}

	return pAction->m_Bindings[index].m_Key;
}

int32 CInputActions::GetBindingMods(const char* pName, int32 index)
{
	const InputAction_t* pAction = FindAction(pName);
	if (!pAction || index < 0 || index >= pAction->m_BindingCount)
	{
		return 0;
	}

	return pAction->m_Bindings[index].m_Mods;
}

static float64 GetEventTime(const char* pName, bool isPress)
{
	const InputAction_t* pAction = FindAction(pName);
	if (!pAction || !s_pActiveInput)
	{
		return 0.0;
	}

	float64 time = 0.0;
	for (int32 event = 0; event < s_pActiveInput->GetKeyEventCount(); event++)
	{
		const bool isMatch = isPress
			? s_pActiveInput->WasKeyEventPressed(event)
			: s_pActiveInput->WasKeyEventReleased(event);
		if (!isMatch)
		{
			continue;
		}

		const int32 key = s_pActiveInput->GetKeyEventKey(event);
		const int32 mods = s_pActiveInput->GetKeyEventMods(event) & s_ModsMask;
		for (int32 i = 0; i < pAction->m_BindingCount; i++)
		{
			const InputBinding_t& binding = pAction->m_Bindings[i];
			if (binding.m_Key == key && (!isPress || binding.m_Mods == mods))
			{
				time = s_pActiveInput->GetKeyEventTime(event);
			}
		}
	}

	return time;
}

bool CInputActions::IsDown(const char* pName)
{
	const InputAction_t* pAction = FindAction(pName);
	if (!pAction || !s_pActiveInput)
	{
		return false;
	}

	const int32 heldMods = GetHeldMods();
	for (int32 i = 0; i < pAction->m_BindingCount; i++)
	{
		if (pAction->m_Bindings[i].m_Mods == heldMods
			&& s_pActiveInput->IsKeyDown(pAction->m_Bindings[i].m_Key))
		{
			return true;
		}
	}

	return false;
}

bool CInputActions::WasPressed(const char* pName)
{
	const InputAction_t* pAction = FindAction(pName);
	if (!pAction || !s_pActiveInput)
	{
		return false;
	}

	for (int32 event = 0; event < s_pActiveInput->GetKeyEventCount(); event++)
	{
		if (!s_pActiveInput->WasKeyEventPressed(event))
		{
			continue;
		}

		const int32 key = s_pActiveInput->GetKeyEventKey(event);
		const int32 mods = s_pActiveInput->GetKeyEventMods(event) & s_ModsMask;
		for (int32 i = 0; i < pAction->m_BindingCount; i++)
		{
			if (pAction->m_Bindings[i].m_Key == key
				&& pAction->m_Bindings[i].m_Mods == mods)
			{
				return true;
			}
		}
	}

	return false;
}

bool CInputActions::WasReleased(const char* pName)
{
	const InputAction_t* pAction = FindAction(pName);
	if (!pAction || !s_pActiveInput)
	{
		return false;
	}

	for (int32 event = 0; event < s_pActiveInput->GetKeyEventCount(); event++)
	{
		if (!s_pActiveInput->WasKeyEventReleased(event))
		{
			continue;
		}

		const int32 key = s_pActiveInput->GetKeyEventKey(event);
		for (int32 i = 0; i < pAction->m_BindingCount; i++)
		{
			if (pAction->m_Bindings[i].m_Key == key)
			{
				return true;
			}
		}
	}

	return false;
}

float64 CInputActions::GetPressedTime(const char* pName)
{
	return GetEventTime(pName, true);
}

float64 CInputActions::GetReleasedTime(const char* pName)
{
	return GetEventTime(pName, false);
}
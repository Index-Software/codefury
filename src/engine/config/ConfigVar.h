//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CONFIGVAR_H
#define CONFIGVAR_H

#pragma once

#include "platform.h"
#include "WrenBind.h"

extern const char* const g_ConfigFilePath;

enum ConfigVarFlags_t
{
	ConfigVarNone = 0,
	ConfigVarArchive = 1 << 0,
	ConfigVarReadOnly = 1 << 1,
	ConfigVarCheat = 1 << 2,
	ConfigVarNeedsApply = 1 << 3
};

struct ConfigVarType_t
{
	enum Enum
	{
		Float,
		Int,
		Bool
	};
};

class CConfigVar
{
public:
	CConfigVar(const char* pName, float32 defaultValue, uint32 flags);
	CConfigVar(
		const char* pName,
		float32 defaultValue,
		uint32 flags,
		float32 minValue,
		float32 maxValue);
	CConfigVar(const char* pName, int32 defaultValue, uint32 flags);
	CConfigVar(
		const char* pName,
		int32 defaultValue,
		uint32 flags,
		int32 minValue,
		int32 maxValue);
	CConfigVar(const char* pName, bool defaultValue, uint32 flags);

	float32 GetFloat() const;
	int32 GetInt() const;
	bool GetBool() const;
	const char* GetName() const;
	uint32 GetFlags() const;
	ConfigVarType_t::Enum GetType() const;
	bool IsReadOnly() const;

	void SetFloat(float32 value);
	void SetInt(int32 value);
	void SetBool(bool value);
	void SetFromString(const char* pValue);

	CConfigVar* GetNext() const;
	static CConfigVar* GetFirst();

	static float32 FindFloat(const char* pName)
		WREN_BIND_STATIC("getFloat(_)");
	static int32 FindInt(const char* pName)
		WREN_BIND_STATIC("getInt(_)");
	static bool FindBool(const char* pName)
		WREN_BIND_STATIC("getBool(_)");
private:
	void Register();
	void Clamp();

	char m_Name[64];
	CConfigVar* m_pNext;
	union
	{
		float32 m_FloatValue;
		int32 m_IntValue;
		bool m_BoolValue;
	};
	float32 m_Min;
	float32 m_Max;
	uint32 m_Flags;
	ConfigVarType_t::Enum m_Type;
	bool m_HasMin;
	bool m_HasMax;
};

class CConfigVarManager
{
public:
	static CConfigVar* Find(const char* pName);
	static bool Load(const char* pFilename);
	static bool Save(const char* pFilename);
};

#endif // CONFIGVAR_H
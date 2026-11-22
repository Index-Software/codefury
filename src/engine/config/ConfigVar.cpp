//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "ConfigVar.h"
#include "dbg.h"
#include "utils/Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* const g_ConfigFilePath = "cfg/config.cfg";

static CConfigVar* s_pFirstVar = CF_NULL;
static CConfigVar* s_pLastVar = CF_NULL;

CConfigVar::CConfigVar(
	const char* pName,
	float32 defaultValue,
	uint32 flags) :
	m_pNext(CF_NULL),
	m_FloatValue(defaultValue),
	m_Min(0.0f),
	m_Max(0.0f),
	m_Flags(flags),
	m_Type(ConfigVarType_t::Float),
	m_HasMin(false),
	m_HasMax(false)
{
	memset(m_Name, 0, sizeof(m_Name));
	strncpy(m_Name, pName, sizeof(m_Name) - 1);

	Register();
}

CConfigVar::CConfigVar(
	const char* pName,
	float32 defaultValue,
	uint32 flags,
	float32 minValue,
	float32 maxValue) :
	m_pNext(CF_NULL),
	m_FloatValue(defaultValue),
	m_Min(minValue),
	m_Max(maxValue),
	m_Flags(flags),
	m_Type(ConfigVarType_t::Float),
	m_HasMin(true),
	m_HasMax(true)
{
	memset(m_Name, 0, sizeof(m_Name));
	strncpy(m_Name, pName, sizeof(m_Name) - 1);

	Register();
}

CConfigVar::CConfigVar(
	const char* pName,
	int32 defaultValue,
	uint32 flags) :
	m_pNext(CF_NULL),
	m_IntValue(defaultValue),
	m_Min(0.0f),
	m_Max(0.0f),
	m_Flags(flags),
	m_Type(ConfigVarType_t::Int),
	m_HasMin(false),
	m_HasMax(false)
{
	memset(m_Name, 0, sizeof(m_Name));
	strncpy(m_Name, pName, sizeof(m_Name) - 1);

	Register();
}

CConfigVar::CConfigVar(
	const char* pName,
	int32 defaultValue,
	uint32 flags,
	int32 minValue,
	int32 maxValue) :
	m_pNext(CF_NULL),
	m_IntValue(defaultValue),
	m_Min(float32(minValue)),
	m_Max(float32(maxValue)),
	m_Flags(flags),
	m_Type(ConfigVarType_t::Int),
	m_HasMin(true),
	m_HasMax(true)
{
	memset(m_Name, 0, sizeof(m_Name));
	strncpy(m_Name, pName, sizeof(m_Name) - 1);

	Register();
}

CConfigVar::CConfigVar(
	const char* pName,
	bool defaultValue,
	uint32 flags) :
	m_pNext(CF_NULL),
	m_BoolValue(defaultValue),
	m_Min(0.0f),
	m_Max(0.0f),
	m_Flags(flags),
	m_Type(ConfigVarType_t::Bool),
	m_HasMin(false),
	m_HasMax(false)
{
	memset(m_Name, 0, sizeof(m_Name));
	strncpy(m_Name, pName, sizeof(m_Name) - 1);

	Register();
}

float32 CConfigVar::GetFloat() const
{
	if (m_Type == ConfigVarType_t::Float)
	{
		return m_FloatValue;
	}

	if (m_Type == ConfigVarType_t::Int)
	{
		return float32(m_IntValue);
	}

	return m_BoolValue ? 1.0f : 0.0f;
}

int32 CConfigVar::GetInt() const
{
	if (m_Type == ConfigVarType_t::Float)
	{
		return int32(m_FloatValue);
	}

	if (m_Type == ConfigVarType_t::Bool)
	{
		return m_BoolValue ? 1 : 0;
	}

	return m_IntValue;
}

bool CConfigVar::GetBool() const
{
	if (m_Type == ConfigVarType_t::Bool)
	{
		return m_BoolValue;
	}

	return GetFloat() != 0.0f;
}

const char* CConfigVar::GetName() const
{
	return m_Name;
}

uint32 CConfigVar::GetFlags() const
{
	return m_Flags;
}

ConfigVarType_t::Enum CConfigVar::GetType() const
{
	return m_Type;
}

bool CConfigVar::IsReadOnly() const
{
	return (m_Flags & ConfigVarReadOnly) != 0;
}

void CConfigVar::SetFloat(float32 value)
{
	if (m_Type != ConfigVarType_t::Float || IsReadOnly())
	{
		return;
	}

	m_FloatValue = value;

	Clamp();
}

void CConfigVar::SetInt(int32 value)
{
	if (m_Type != ConfigVarType_t::Int || IsReadOnly())
	{
		return;
	}

	m_IntValue = value;

	Clamp();
}

void CConfigVar::SetBool(bool value)
{
	if (m_Type != ConfigVarType_t::Bool || IsReadOnly())
	{
		return;
	}

	m_BoolValue = value;
}

void CConfigVar::SetFromString(const char* pValue)
{
	if (m_Type == ConfigVarType_t::Float)
	{
		SetFloat(float32(atof(pValue)));
	}
	else if (m_Type == ConfigVarType_t::Int)
	{
		SetInt(int32(atoi(pValue)));
	}
	else
	{
		SetBool(strcmp(pValue, "true") == 0 || strcmp(pValue, "1") == 0);
	}
}

CConfigVar* CConfigVar::GetNext() const
{
	return m_pNext;
}

CConfigVar* CConfigVar::GetFirst()
{
	return s_pFirstVar;
}

float32 CConfigVar::FindFloat(const char* pName)
{
	CConfigVar* pConfigVar = CConfigVarManager::Find(pName);
	if (!pConfigVar)
	{
		Warning("[CConfigVar] CVar %s not found\n", pName);

		return 0.0f;
	}

	return pConfigVar->GetFloat();
}

int32 CConfigVar::FindInt(const char* pName)
{
	CConfigVar* pConfigVar = CConfigVarManager::Find(pName);
	if (!pConfigVar)
	{
		Warning("[CConfigVar] CVar %s not found\n", pName);

		return 0;
	}

	return pConfigVar->GetInt();
}

bool CConfigVar::FindBool(const char* pName)
{
	CConfigVar* pConfigVar = CConfigVarManager::Find(pName);
	if (!pConfigVar)
	{
		Warning("[CConfigVar] CVar %s not found\n", pName);

		return false;
	}

	return pConfigVar->GetBool();
}

void CConfigVar::Register()
{
	m_pNext = CF_NULL;

	if (s_pLastVar)
	{
		s_pLastVar->m_pNext = this;
	}
	else
	{
		s_pFirstVar = this;
	}

	s_pLastVar = this;
}

void CConfigVar::Clamp()
{
	if (m_Type == ConfigVarType_t::Float)
	{
		if (m_HasMin && m_FloatValue < m_Min)
		{
			m_FloatValue = m_Min;
		}

		if (m_HasMax && m_FloatValue > m_Max)
		{
			m_FloatValue = m_Max;
		}
	}
	else if (m_Type == ConfigVarType_t::Int)
	{
		if (m_HasMin && m_IntValue < int32(m_Min))
		{
			m_IntValue = int32(m_Min);
		}

		if (m_HasMax && m_IntValue > int32(m_Max))
		{
			m_IntValue = int32(m_Max);
		}
	}
}

CConfigVar* CConfigVarManager::Find(const char* pName)
{
	for (CConfigVar* pVar = CConfigVar::GetFirst(); pVar;
		pVar = pVar->GetNext())
	{
		if (strcmp(pVar->GetName(), pName) == 0)
		{
			return pVar;
		}
	}

	return CF_NULL;
}

bool CConfigVarManager::Load(const char* pFilename)
{
	CFixedString contents = CUtils::ReadFile(CFixedString(pFilename));
	if (contents.IsEmpty())
	{
		return false;
	}

	const char* pData = contents.c_str();
	char line[256];
	int32 unknownCount = 0;

	while (pData && *pData)
	{
		const char* pLineEnd = strchr(pData, '\n');
		uint32 lineLength = pLineEnd
			? uint32(pLineEnd - pData)
			: uint32(strlen(pData));
		if (lineLength >= sizeof(line))
		{
			lineLength = uint32(sizeof(line) - 1);
		}
		memcpy(line, pData, lineLength);
		line[lineLength] = '\0';
		pData = pLineEnd ? pLineEnd + 1 : pData + strlen(pData);

		char name[64];
		char value[64];

		if (line[0] == '#' || line[0] == '/' ||
			sscanf(line, "%63s %63s", name, value) != 2)
		{
			continue;
		}

		CConfigVar* pVar = Find(name);
		if (pVar)
		{
			pVar->SetFromString(value);
		}
		else
		{
			unknownCount++;
		}
	}

	if (unknownCount > 0)
	{
		Warning(
			"[CConfigVarManager] Skipped %d unknown config entries\n",
			unknownCount);
	}

	return true;
}

bool CConfigVarManager::Save(const char* pFilename)
{
	CFixedString contents;
	char line[128];

	for (CConfigVar* pVar = CConfigVar::GetFirst(); pVar;
		pVar = pVar->GetNext())
	{
		if (!(pVar->GetFlags() & ConfigVarArchive))
		{
			continue;
		}

		if (pVar->GetType() == ConfigVarType_t::Float)
		{
			sprintf(line, "%s %g\n", pVar->GetName(), pVar->GetFloat());
		}
		else if (pVar->GetType() == ConfigVarType_t::Int)
		{
			sprintf(line, "%s %d\n", pVar->GetName(), pVar->GetInt());
		}
		else
		{
			sprintf(
				line,
				"%s %s\n",
				pVar->GetName(),
				pVar->GetBool() ? "true" : "false");
		}

		contents = contents + line;
	}

	return CUtils::WriteFile(CFixedString(pFilename), contents);
}
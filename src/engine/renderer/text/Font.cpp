//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Font.h"
#include "dbg.h"
#include "utils/KeyValueLine.h"
#include "utils/StringUtils.h"
#include "utils/Utils.h"
#include <string.h>

CFont::CFont() :
	m_AtlasWidth(0),
	m_AtlasHeight(0),
	m_EmSize(0.0f),
	m_DistanceRange(0.0f)
{
	m_Metrics.m_Ascent = 0.0f;
	m_Metrics.m_Descent = 0.0f;
	m_Metrics.m_LineHeight = 0.0f;
}

bool CFont::LoadFromFile(
	IRenderer* pRenderer,
	const char* pAtlasFilename,
	const char* pMetricsFilename)
{
	m_pAtlasTexture = CRef<CTexture>::Create(pRenderer);
	if (!m_pAtlasTexture.IsValid())
	{
		return false;
	}

	if (!m_pAtlasTexture->LoadFromFile(pAtlasFilename))
	{
		Error("[CFont] Failed to load atlas texture: %s\n", pAtlasFilename);

		m_pAtlasTexture.Reset();

		return false;
	}

	m_AtlasWidth = m_pAtlasTexture->GetWidth();
	m_AtlasHeight = m_pAtlasTexture->GetHeight();

	if (!LoadMetrics(pMetricsFilename))
	{
		m_pAtlasTexture.Reset();

		return false;
	}

	Msg(
		"Font %s loaded: %d glyphs, atlas %ux%u, em %.1f, "
		"distance range %.1f\n",
		pMetricsFilename,
		m_Glyphs.Count(),
		m_AtlasWidth,
		m_AtlasHeight,
		m_EmSize,
		m_DistanceRange);

	return true;
}

uint32 CFont::GetAtlasWidth() const
{
	return m_AtlasWidth;
}

uint32 CFont::GetAtlasHeight() const
{
	return m_AtlasHeight;
}

const Glyph_t* CFont::GetGlyph(uint32 codepoint) const
{
	int32 index = m_Glyphs.Find(codepoint);
	if (index == m_Glyphs.InvalidIndex())
	{
		return CF_NULL;
	}

	return &m_Glyphs.Element(index);
}

const FontMetrics_t& CFont::GetMetrics() const
{
	return m_Metrics;
}

bool CFont::LoadMetrics(const char* pFilename)
{
	CFixedString contents = CUtils::ReadFile(CFixedString(pFilename));
	if (contents.IsEmpty())
	{
		Error("[CFont] Failed to read metrics file: %s\n", pFilename);

		return false;
	}

	m_Glyphs.RemoveAll();

	const char* pText = contents.c_str();
	char line[256];
	int32 lineNumber = 0;
	bool hasFontLine = false;

	while (CStringUtils::GetLine(pText, line, sizeof(line)))
	{
		lineNumber++;

		CKeyValueLine keyValues;
		if (!keyValues.Parse(line))
		{
			Error("[CFont] Malformed line %d: %s\n", lineNumber, pFilename);

			return false;
		}

		if (keyValues.IsEmpty())
		{
			continue;
		}

		if (strcmp(keyValues.GetTag(), "font") == 0)
		{
			m_EmSize = keyValues.GetFloat("em");
			m_Metrics.m_Ascent = keyValues.GetFloat("ascent");
			m_Metrics.m_Descent = keyValues.GetFloat("descent");
			m_Metrics.m_LineHeight = keyValues.GetFloat("lineHeight");
			m_DistanceRange = keyValues.GetFloat("distanceRange");

			int32 atlasWidth = keyValues.GetInt("atlasWidth");
			int32 atlasHeight = keyValues.GetInt("atlasHeight");

			if (!keyValues.GetInvalidKey() &&
				(uint32(atlasWidth) != m_AtlasWidth ||
				uint32(atlasHeight) != m_AtlasHeight))
			{
				Error(
					"[CFont] Metrics do not match the atlas: %s\n",
					pFilename);

				return false;
			}

			hasFontLine = true;
		}
		else if (strcmp(keyValues.GetTag(), "glyph") == 0)
		{
			uint32 codepoint = uint32(keyValues.GetInt("codepoint"));

			Glyph_t entry;
			entry.m_Advance = keyValues.GetFloat("advance");
			entry.m_OffsetX = keyValues.GetFloat("offsetX");
			entry.m_OffsetY = keyValues.GetFloat("offsetY");
			entry.m_AtlasX = keyValues.GetInt("atlasX");
			entry.m_AtlasY = keyValues.GetInt("atlasY");
			entry.m_Width = keyValues.GetInt("width");
			entry.m_Height = keyValues.GetInt("height");

			if (!keyValues.GetInvalidKey())
			{
				m_Glyphs.Insert(codepoint, entry);
			}
		}

		if (keyValues.GetInvalidKey())
		{
			Error(
				"[CFont] Missing or invalid \"%s\" on line %d: %s\n",
				keyValues.GetInvalidKey(),
				lineNumber,
				pFilename);

			return false;
		}
	}

	if (!hasFontLine || m_Glyphs.Count() == 0)
	{
		Error(
			"[CFont] No font or glyph lines in metrics file: %s\n",
			pFilename);

		return false;
	}

	return true;
}
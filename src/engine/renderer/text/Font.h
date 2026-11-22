//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef FONT_H
#define FONT_H

#pragma once

#include "renderer/Renderer.h"
#include "renderer/texture/Texture.h"
#include "utils/FixedString.h"

typedef uint32 CFontHandle;

static const CFontHandle s_InvalidFontHandle = 0;

struct Glyph_t
{
	int32 m_AtlasX, m_AtlasY;
	int32 m_Width, m_Height;
	float32 m_OffsetX, m_OffsetY;
	float32 m_Advance;
};

struct FontMetrics_t
{
	float32 m_Ascent;
	float32 m_Descent;
	float32 m_LineHeight;
};

class CFont
{
public:
	CFont();

	bool LoadFromFile(
		IRenderer* pRenderer,
		const char* pAtlasFilename,
		const char* pMetricsFilename);

	CTexture* GetAtlas() const
	{
		return m_pAtlasTexture.Get();
	}

	uint32 GetAtlasWidth() const;
	uint32 GetAtlasHeight() const;
	const Glyph_t* GetGlyph(uint32 codepoint) const;
	const FontMetrics_t& GetMetrics() const;

	float32 GetEmSize() const
	{
		return m_EmSize;
	}

	float32 GetDistanceRange() const
	{
		return m_DistanceRange;
	}
private:
	bool LoadMetrics(const char* pFilename);

	CUtlMap<uint32, Glyph_t> m_Glyphs;
	FontMetrics_t m_Metrics;
	CRef<CTexture> m_pAtlasTexture;
	uint32 m_AtlasWidth;
	uint32 m_AtlasHeight;
	float32 m_EmSize;
	float32 m_DistanceRange;
};

#endif // FONT_H
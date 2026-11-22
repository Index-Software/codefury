//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#pragma once

#include "renderer/text/Font.h"
#include "renderer/BatchPool.h"
#include "renderer/Renderer.h"
#include "renderer/material/Material.h"
#include "utils/FixedString.h"
#include "utils/UtlMap.h"
#include "utils/UtlVector.h"

class CMatrix4;

struct TextVertex_t
{
	float32 m_X, m_Y, m_Z;
	uint32 m_Color;
	float32 m_U, m_V;
	uint32 m_OutlineColor;
	float32 m_EdgeSharpness;
};

class CTextRenderer
{
public:
	CTextRenderer(IRenderer* pRenderer);
	~CTextRenderer();

	bool Init(float32 fontSize, float32 textSoftness);
	void Shutdown();

	void BeginTextBatch(const CMatrix4& projection);

	void RenderGlyphs(
		int32 vertexStart,
		int32 vertexEnd,
		CFontHandle hFont);

	int32 GetVertexCount() const
	{
		return m_Vertices.Count();
	}

	void DrawText(
		const char* pText,
		float32 x,
		float32 y,
		const CColor& color,
		const CColor& outlineColor,
		float32 outlineWidth,
		float32 softness);

	CFontHandle LoadFont(
		const char* pName,
		const char* pAtlasFilename,
		const char* pMetricsFilename);
	bool SetFont(CFontHandle hFont);
	bool SetFontByName(const char* pName);
	void SetDefaultFont(CFontHandle hFont);
	void ResetFont();

	CFontHandle GetCurrentFont() const
	{
		return m_hCurrentFont;
	}

	CFont* GetFont(CFontHandle hFont) const;

	float32 MeasureTextWidth(const char* pText) const;
	float32 MeasureTextHeight(const char* pText) const;

	float32 GetTextLineHeight() const;
	float32 GetTextAscent() const;
	float32 GetTextDescent() const;

	void SetFontSize(float32 size);

	void SetTextSoftness(float32 softness);
	float32 GetTextSoftness() const
	{
		return m_TextSoftness;
	}
	float32 GetFontSize() const;
	void SetLetterSpacing(float32 spacing);
	float32 GetLetterSpacing() const;
	void SetLineSpacing(float32 spacing);
	float32 GetLineSpacing() const;

	static const float32 s_MinSoftness;
	static const float32 s_MaxSoftness;
private:
	float32 GetAtlasScale() const;

	void AddGlyphToBatch(
		const Glyph_t* pGlyph,
		float32 x,
		float32 y,
		float32 scale,
		uint32 packedColor,
		uint32 packedOutlineColor,
		float32 edgeSharpness);

	CBatchPool m_BatchPool;
	CVertexLayout m_TextLayout;

	CUtlVector<CFont*> m_Fonts;
	CUtlMap<CFixedString, CFontHandle> m_FontNames;
	CUtlVector<TextVertex_t> m_Vertices;

	IRenderer* m_pRenderer;
	CFont* m_pCurrentFont;
	CRef<CMaterial> m_pCachedMaterial;

	BufferHandle m_hTextUniformBuffer;

	UniformBlockLayoutHandle m_hTextUniformLayout;
	uint32 m_SlotVertexCapacity;
	CFontHandle m_hCurrentFont;
	CFontHandle m_hDefaultFont;
	CFontHandle m_hCachedFont;

	float32 m_TextSoftness;
	float32 m_CurrentFontSize;
	float32 m_LetterSpacing;
	float32 m_LineSpacing;
};

#endif // TEXT_RENDERER_H
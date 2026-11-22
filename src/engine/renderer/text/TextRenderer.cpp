//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "TextRenderer.h"
#include "dbg.h"
#include "mathsfury/Maths.h"
#include "mathsfury/Vector4.h"
#include "renderer/material/Material.h"
#include "renderer/shader/Shader.h"
#include "renderer/shader/ShaderTypes.h"
#include "renderer/texture/Texture.h"
#include "shaders/ShaderConstants.h"
#include "ecs/components/Mesh.h"
#include "renderer/RenderQueue.h"
#include "utils/Time.h"
#include "utils/UtlMemory.h"

static const uint32 s_InitialTextBatchSlots = 8;

static const float32 s_MinOutlineWidth = 1.0f;

const float32 CTextRenderer::s_MinSoftness = 0.5f;
const float32 CTextRenderer::s_MaxSoftness = 2.0f;

CTextRenderer::CTextRenderer(IRenderer* pRenderer) :
	m_pRenderer(pRenderer),
	m_pCurrentFont(CF_NULL),
	m_hTextUniformBuffer(0),
	m_hTextUniformLayout(0),
	m_SlotVertexCapacity(4096),
	m_hCurrentFont(s_InvalidFontHandle),
	m_hDefaultFont(s_InvalidFontHandle),
	m_hCachedFont(s_InvalidFontHandle),
	m_TextSoftness(1.0f),
	m_CurrentFontSize(0.0f),
	m_LetterSpacing(0.0f),
	m_LineSpacing(1.0f)
{
}

CTextRenderer::~CTextRenderer()
{
	Shutdown();
}

bool CTextRenderer::Init(float32 fontSize, float32 textSoftness)
{
	SetFontSize(fontSize);
	SetTextSoftness(textSoftness);

	m_Vertices.EnsureCapacity(m_SlotVertexCapacity);

	if (!m_BatchPool.Init(
		m_pRenderer,
		m_SlotVertexCapacity * sizeof(TextVertex_t),
		m_SlotVertexCapacity / 4,
		s_InitialTextBatchSlots,
		m_pRenderer->GetMaxFrameLatency()))
	{
		Error("[CTextRenderer] Failed to create batch buffers\n");

		Shutdown();

		return false;
	}

	m_TextLayout.SetStride(sizeof(TextVertex_t));
	m_TextLayout.AddAttribute(
		"position",
		static_cast<uint32>(VertexFormat_t::Float3),
		0,
		VertexSemantic_t::Position);
	m_TextLayout.AddAttribute(
		"color",
		static_cast<uint32>(VertexFormat_t::UByte4_Norm),
		12,
		VertexSemantic_t::Color0);
	m_TextLayout.AddAttribute(
		"texCoord",
		static_cast<uint32>(VertexFormat_t::Float2),
		16,
		VertexSemantic_t::TexCoord0);
	m_TextLayout.AddAttribute(
		"outlineColor",
		static_cast<uint32>(VertexFormat_t::UByte4_Norm),
		24,
		VertexSemantic_t::TexCoord1);
	m_TextLayout.AddAttribute(
		"edgeSharpness",
		static_cast<uint32>(VertexFormat_t::Float),
		28,
		VertexSemantic_t::TexCoord3);

	UniformBlockLayout_t modelLayout = CShaderLayouts::GetModelLayout();
	m_hTextUniformLayout = m_pRenderer->CreateUniformBlockLayout(modelLayout);

	m_hTextUniformBuffer = m_pRenderer->CreateConstantBuffer(
		modelLayout.m_Size,
		BufferUsage_t::Dynamic);

	if (m_hTextUniformLayout == 0 || m_hTextUniformBuffer == 0)
	{
		Error("[CTextRenderer] Failed to create uniform resources\n");

		Shutdown();

		return false;
	}

	CRef<CShader> pShader = CRef<CShader>::Create(m_pRenderer);
	if (!pShader.IsValid() || !pShader->CreateFromSource("ui/Text"))
	{
		Error("[CTextRenderer] Failed to create the text shader\n");

		Shutdown();

		return false;
	}

	m_pCachedMaterial = CRef<CMaterial>::Create(m_pRenderer);
	if (!m_pCachedMaterial.IsValid())
	{
		Error("[CTextRenderer] Failed to create the text material\n");

		Shutdown();

		return false;
	}

	m_pCachedMaterial->SetShader(pShader.Get());
	m_pCachedMaterial->SetRenderQueue(RenderQueue_t::Overlay);

	return true;
}

void CTextRenderer::Shutdown()
{
	for (int32 i = 0; i < m_Fonts.Count(); i++)
	{
		CFont* pFont = m_Fonts[i];
		pFont->~CFont();
		CUtlMemory::Free(pFont);
	}
	m_Fonts.RemoveAll();
	m_FontNames.RemoveAll();
	m_pCurrentFont = CF_NULL;
	m_hCurrentFont = s_InvalidFontHandle;
	m_hDefaultFont = s_InvalidFontHandle;
	m_hCachedFont = s_InvalidFontHandle;

	m_BatchPool.Shutdown();

	if (m_hTextUniformBuffer)
	{
		m_pRenderer->DestroyBuffer(m_hTextUniformBuffer);
		m_hTextUniformBuffer = 0;
	}

	if (m_hTextUniformLayout)
	{
		m_pRenderer->DestroyUniformBlockLayout(m_hTextUniformLayout);
		m_hTextUniformLayout = 0;
	}

	m_pCachedMaterial.Reset();

	m_Vertices.RemoveAll();
}

void CTextRenderer::BeginTextBatch(const CMatrix4& projection)
{
	m_Vertices.RemoveAll();

	CfModel_t modelData;
	modelData.m_MVP = projection;
	modelData.m_Model = CMatrix4(1.0f);

	m_pRenderer->UpdateBuffer(
		m_hTextUniformBuffer,
		&modelData,
		sizeof(CfModel_t),
		0);
}

void CTextRenderer::RenderGlyphs(
	int32 vertexStart,
	int32 vertexEnd,
	CFontHandle hFont)
{
	if (vertexStart >= vertexEnd)
	{
		return;
	}

	CFont* pFont = GetFont(hFont);
	if (!pFont)
	{
		return;
	}

	if (!m_pCachedMaterial.IsValid())
	{
		Error("[CTextRenderer] Text batch resources are not initialized\n");

		return;
	}

	if (m_hCachedFont != hFont)
	{
		m_pCachedMaterial->SetTexture(pFont->GetAtlas());
		m_hCachedFont = hFont;
	}

	m_pRenderer->SetShaderProgram(
		m_pCachedMaterial->GetShader()->GetHandle());

	m_pRenderer->SetConstantBuffer(
		m_hTextUniformBuffer,
		m_hTextUniformLayout,
		CF_BINDING_MODEL,
		ShaderStageVertex);

	BlendState_t blendState;
	blendState.m_IsEnabled = true;
	blendState.m_IsAdditive = false;
	m_pRenderer->SetBlendState(blendState);

	DepthStencilState_t depthState;
	depthState.m_IsDepthTest = false;
	depthState.m_IsDepthWrite = false;
	m_pRenderer->SetDepthStencilState(depthState);

	if (m_pCachedMaterial.IsValid() && m_pCachedMaterial->GetTexture())
	{
		m_pRenderer->SetTexture(
			m_pCachedMaterial->GetTexture()->GetHandle(),
			0,
			ShaderStageFragment,
			m_pCachedMaterial->GetTexture()->GetWrapMode());
	}

	int32 startGlyph = vertexStart / 4;
	int32 endGlyph = vertexEnd / 4;
	int32 glyphsPerSlot = int32(m_SlotVertexCapacity) / 4;

	int32 glyphStart = startGlyph;
	while (glyphStart < endGlyph)
	{
		BufferHandle hVertexBuffer = 0;
		if (!m_BatchPool.GetBatch(hVertexBuffer))
		{
			Error("[CTextRenderer] Failed to get a batch buffer\n");

			return;
		}

		int32 chunkGlyphs = endGlyph - glyphStart;
		if (chunkGlyphs > glyphsPerSlot)
		{
			chunkGlyphs = glyphsPerSlot;
		}

		int32 glyphEnd = glyphStart + chunkGlyphs;

		m_pRenderer->UpdateBuffer(
			hVertexBuffer,
			m_Vertices.Base() + glyphStart * 4,
			chunkGlyphs * 4 * sizeof(TextVertex_t),
			0);

		m_pRenderer->SetVertexBuffer(
			hVertexBuffer,
			0,
			m_TextLayout.GetStride(),
			0,
			&m_TextLayout);
		m_pRenderer->SetIndexBuffer(m_BatchPool.GetQuadIndexBuffer(), 0);

		m_pRenderer->DrawIndexed(
			static_cast<uint32>(chunkGlyphs * 6),
			0,
			0);

		glyphStart = glyphEnd;
	}
}

void CTextRenderer::DrawText(
	const char* pText,
	float32 x,
	float32 y,
	const CColor& color,
	const CColor& outlineColor,
	float32 outlineWidth,
	float32 softness)
{
	if (!m_pCurrentFont)
	{
		Warning("[CTextRenderer] DrawText called but no current font set\n");

		return;
	}

	float32 scale = GetAtlasScale();
	float32 screenPxRange = m_pCurrentFont->GetDistanceRange() * scale;
	float32 edgeWidth = softness;
	float32 outlinePixels = outlineWidth * m_CurrentFontSize;

	if (outlinePixels > 0.0f && outlinePixels < s_MinOutlineWidth)
	{
		outlinePixels = s_MinOutlineWidth;
	}

	float32 maxOutline = screenPxRange * 0.5f - edgeWidth * 0.5f;
	if (outlinePixels > maxOutline)
	{
		outlinePixels = maxOutline;
	}

	if (outlinePixels < edgeWidth * 0.5f)
	{
		outlinePixels = 0.0f;
	}

	float32 edgeSharpness = screenPxRange / edgeWidth;
	float32 outlineEdges = outlinePixels / edgeWidth;
	float32 outlineRange = float32(CF_TEXT_OUTLINE_WIDTH_RANGE);
	if (outlineEdges > outlineRange)
	{
		outlineEdges = outlineRange;
	}

	uint32 packedColor = CColor::PackColor(color);
	uint32 packedOutlineColor = CColor::PackColor(CColor(
		outlineColor.m_R,
		outlineColor.m_G,
		outlineColor.m_B,
		outlineEdges / outlineRange));

	float32 currentX = x;
	float32 currentY = y;

	for (int32 i = 0; pText[i] != '\0'; i++)
	{
		char c = pText[i];
		if (c == '\n')
		{
			currentX = x;
			currentY -= m_pCurrentFont->GetMetrics().m_LineHeight
				* scale * m_LineSpacing;

			continue;
		}

		const Glyph_t* pGlyph = m_pCurrentFont->GetGlyph(
			static_cast<uint32>(c));
		if (!pGlyph)
		{
			continue;
		}

		AddGlyphToBatch(
			pGlyph,
			currentX,
			currentY,
			scale,
			packedColor,
			packedOutlineColor,
			edgeSharpness);

		currentX += pGlyph->m_Advance * scale + m_LetterSpacing;
	}
}

CFontHandle CTextRenderer::LoadFont(
	const char* pName,
	const char* pAtlasFilename,
	const char* pMetricsFilename)
{
	if (!pName || !pAtlasFilename || !pMetricsFilename)
	{
		Error("[CTextRenderer] Font name, atlas and metrics are required\n");

		return s_InvalidFontHandle;
	}

	if (m_FontNames.Find(CFixedString(pName)) != m_FontNames.InvalidIndex())
	{
		Error("[CTextRenderer] Font %s is already loaded\n", pName);

		return s_InvalidFontHandle;
	}

	void* pMemory = CUtlMemory::Alloc(sizeof(CFont));
	if (!pMemory)
	{
		return s_InvalidFontHandle;
	}

	CFont* pFont = new(pMemory) CFont();
	if (!pFont->LoadFromFile(m_pRenderer, pAtlasFilename, pMetricsFilename))
	{
		Error("[CTextRenderer] Failed to load font %s\n", pAtlasFilename);

		pFont->~CFont();
		CUtlMemory::Free(pFont);

		return s_InvalidFontHandle;
	}

	if (!pFont->GetAtlas())
	{
		Error("[CTextRenderer] Font has no texture atlas texture\n");

		pFont->~CFont();
		CUtlMemory::Free(pFont);

		return s_InvalidFontHandle;
	}

	m_Fonts.AddToTail(pFont);

	CFontHandle hFont = static_cast<CFontHandle>(m_Fonts.Count());
	m_FontNames.Insert(CFixedString(pName), hFont);

	return hFont;
}

bool CTextRenderer::SetFont(CFontHandle hFont)
{
	CFont* pFont = GetFont(hFont);
	if (!pFont)
	{
		Warning("[CTextRenderer] Invalid font handle %u\n", hFont);

		return false;
	}

	m_hCurrentFont = hFont;
	m_pCurrentFont = pFont;

	return true;
}

bool CTextRenderer::SetFontByName(const char* pName)
{
	if (!pName)
	{
		return false;
	}

	int32 index = m_FontNames.Find(CFixedString(pName));
	if (index == m_FontNames.InvalidIndex())
	{
		Warning("[CTextRenderer] Font %s not found\n", pName);

		return false;
	}

	return SetFont(m_FontNames.Element(index));
}

void CTextRenderer::SetDefaultFont(CFontHandle hFont)
{
	if (!GetFont(hFont))
	{
		Warning("[CTextRenderer] Invalid font handle %u\n", hFont);

		return;
	}

	m_hDefaultFont = hFont;
}

void CTextRenderer::ResetFont()
{
	m_hCurrentFont = m_hDefaultFont;
	m_pCurrentFont = GetFont(m_hDefaultFont);
}

CFont* CTextRenderer::GetFont(CFontHandle hFont) const
{
	if (hFont == s_InvalidFontHandle ||
		hFont > static_cast<CFontHandle>(m_Fonts.Count()))
	{
		return CF_NULL;
	}

	return m_Fonts[static_cast<int32>(hFont) - 1];
}

float32 CTextRenderer::MeasureTextWidth(const char* pText) const
{
	if (!m_pCurrentFont)
	{
		return 0.0f;
	}

	float32 scale = GetAtlasScale();

	float32 maxX = 0.0f;
	float32 lineWidth = 0.0f;
	bool isLineStart = true;

	for (int32 i = 0; pText[i] != '\0'; i++)
	{
		char c = pText[i];
		if (c == '\n')
		{
			maxX = maxX > lineWidth ? maxX : lineWidth;
			lineWidth = 0.0f;
			isLineStart = true;

			continue;
		}

		const Glyph_t* pGlyph = m_pCurrentFont->GetGlyph(
			static_cast<uint32>(c));
		if (!pGlyph)
		{
			continue;
		}

		if (!isLineStart)
		{
			lineWidth += m_LetterSpacing;
		}

		lineWidth += pGlyph->m_Advance * scale;
		isLineStart = false;
	}

	maxX = maxX > lineWidth ? maxX : lineWidth;

	return maxX;
}

float32 CTextRenderer::MeasureTextHeight(const char* pText) const
{
	if (!m_pCurrentFont)
	{
		return 0.0f;
	}

	int32 lines = 1;
	for (int32 i = 0; pText[i] != '\0'; i++)
	{
		if (pText[i] == '\n')
		{
			lines++;
		}
	}

	float32 lineHeight = m_pCurrentFont->GetMetrics().m_LineHeight
		* GetAtlasScale();

	return lineHeight + float32(lines - 1) * lineHeight * m_LineSpacing;
}

float32 CTextRenderer::GetTextLineHeight() const
{
	if (!m_pCurrentFont)
	{
		return 0.0f;
	}

	return m_pCurrentFont->GetMetrics().m_LineHeight * GetAtlasScale();
}

float32 CTextRenderer::GetTextAscent() const
{
	if (!m_pCurrentFont)
	{
		return 0.0f;
	}

	return m_pCurrentFont->GetMetrics().m_Ascent * GetAtlasScale();
}

float32 CTextRenderer::GetTextDescent() const
{
	if (!m_pCurrentFont)
	{
		return 0.0f;
	}

	return m_pCurrentFont->GetMetrics().m_Descent * GetAtlasScale();
}

void CTextRenderer::SetFontSize(float32 size)
{
	m_CurrentFontSize = size;
}

void CTextRenderer::SetTextSoftness(float32 softness)
{
	if (softness < s_MinSoftness)
	{
		softness = s_MinSoftness;
	}
	else if (softness > s_MaxSoftness)
	{
		softness = s_MaxSoftness;
	}

	m_TextSoftness = softness;
}

float32 CTextRenderer::GetFontSize() const
{
	return m_CurrentFontSize;
}

void CTextRenderer::SetLetterSpacing(float32 spacing)
{
	m_LetterSpacing = spacing;
}

float32 CTextRenderer::GetLetterSpacing() const
{
	return m_LetterSpacing;
}

void CTextRenderer::SetLineSpacing(float32 spacing)
{
	m_LineSpacing = spacing;
}

float32 CTextRenderer::GetLineSpacing() const
{
	return m_LineSpacing;
}

float32 CTextRenderer::GetAtlasScale() const
{
	if (!m_pCurrentFont)
	{
		return 0.0f;
	}

	float32 emSize = m_pCurrentFont->GetEmSize();
	if (emSize <= 0.0f)
	{
		emSize = 1.0f;
	}

	return m_CurrentFontSize / emSize;
}

void CTextRenderer::AddGlyphToBatch(
	const Glyph_t* pGlyph,
	float32 x,
	float32 y,
	float32 scale,
	uint32 packedColor,
	uint32 packedOutlineColor,
	float32 edgeSharpness)
{
	if (!pGlyph)
	{
		Warning("[CTextRenderer] Failed to add null glyph to batch\n");

		return;
	}

	float32 glyphX = x + pGlyph->m_OffsetX * scale;
	float32 glyphY = y - pGlyph->m_OffsetY * scale;
	float32 glyphW = pGlyph->m_Width * scale;
	float32 glyphH = pGlyph->m_Height * scale;

	float32 atlasWidth = static_cast<float32>(
		m_pCurrentFont->GetAtlasWidth());
	float32 atlasHeight = static_cast<float32>(
		m_pCurrentFont->GetAtlasHeight());

	const float32 texelInset = 0.5f;
	float32 u0 = (pGlyph->m_AtlasX + texelInset) / atlasWidth;
	float32 v0 = (pGlyph->m_AtlasY + pGlyph->m_Height - texelInset)
		/ atlasHeight;
	float32 u1 = (pGlyph->m_AtlasX + pGlyph->m_Width - texelInset) / atlasWidth;
	float32 v1 = (pGlyph->m_AtlasY + texelInset) / atlasHeight;

	float32 textZ = 0.0f;
	TextVertex_t verts[4] =
	{
		{
			glyphX, glyphY, textZ, packedColor, u0, v0,
			packedOutlineColor, edgeSharpness
		},
		{
			glyphX + glyphW, glyphY, textZ, packedColor, u1, v0,
			packedOutlineColor, edgeSharpness
		},
		{
			glyphX + glyphW, glyphY + glyphH, textZ, packedColor, u1, v1,
			packedOutlineColor, edgeSharpness
		},
		{
			glyphX, glyphY + glyphH, textZ, packedColor, u0, v1,
			packedOutlineColor, edgeSharpness
		}
	};

	for (int32 i = 0; i < 4; i++)
	{
		m_Vertices.AddToTail(verts[i]);
	}
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "UI.h"
#include "dbg.h"
#include "config/ConfigVar.h"
#include "renderer/material/Material.h"
#include "renderer/shader/Shader.h"
#include "renderer/text/TextRenderer.h"
#include "renderer/ui/UIBatch.h"
#include "renderer/Renderer.h"
#include "utils/UtlMemory.h"
#include "Matrix4.h"
#include "Maths.h"

static CConfigVar s_rFontSize(
	"r_fontsize",
	24.0f,
	ConfigVarArchive,
	4.0f,
	96.0f);

static CConfigVar s_rTextSoftness(
	"r_textsoftness",
	0.5f,
	ConfigVarArchive,
	CTextRenderer::s_MinSoftness,
	CTextRenderer::s_MaxSoftness);

CUI::CUI() :
	m_pRenderer(CF_NULL),
	m_pTextRenderer(CF_NULL)
{
}

CUI::~CUI()
{
	Shutdown();
}

bool CUI::Init(IRenderer& renderer)
{
	m_pRenderer = &renderer;

	void* pTextMem = CUtlMemory::Alloc(sizeof(CTextRenderer));
	if (!pTextMem)
	{
		return false;
	}

	m_pTextRenderer = new(pTextMem) CTextRenderer(&renderer);
	if (!m_pTextRenderer->Init(
		s_rFontSize.GetFloat(),
		s_rTextSoftness.GetFloat()))
	{
		m_pTextRenderer->~CTextRenderer();
		CUtlMemory::Free(pTextMem);
		m_pTextRenderer = CF_NULL;

		return false;
	}

	m_pUnlitShader = CRef<CShader>::Create(&renderer);
	if (!m_pUnlitShader.IsValid() ||
		!m_pUnlitShader->CreateFromSource("ui/Unlit"))
	{
		Error("[CUI::Init] Failed to create the UI shader\n");

		Shutdown();

		return false;
	}

	m_pDefaultMaterial = CRef<CMaterial>::Create(&renderer);
	if (!m_pDefaultMaterial.IsValid())
	{
		Shutdown();

		return false;
	}

	m_pDefaultMaterial->SetShader(m_pUnlitShader.Get());

	if (!m_Batch.Init(renderer, m_pTextRenderer, m_pDefaultMaterial.Get()))
	{
		Shutdown();

		return false;
	}

	return true;
}

void CUI::Shutdown()
{
	m_Batch.Shutdown();

	m_pDefaultMaterial.Reset();
	m_pUnlitShader.Reset();

	if (m_pTextRenderer)
	{
		m_pTextRenderer->Shutdown();
		m_pTextRenderer->~CTextRenderer();
		CUtlMemory::Free(m_pTextRenderer);
		m_pTextRenderer = CF_NULL;
	}

	m_pRenderer = CF_NULL;
}

Rect_t CUI::GetViewport() const
{
	return Rect_t(
		0,
		0,
		m_Batch.GetViewportWidth(),
		m_Batch.GetViewportHeight());
}

uint32 CUI::GetViewportWidth() const
{
	return m_Batch.GetViewportWidth();
}

uint32 CUI::GetViewportHeight() const
{
	return m_Batch.GetViewportHeight();
}

void CUI::SetDrawColor(const CColor& color)
{
	m_Batch.SetDrawColor(color);
}

CColor CUI::GetDrawColor() const
{
	return m_Batch.GetDrawColor();
}

void CUI::SetMaterial(CMaterial* pMaterial)
{
	m_Batch.SetMaterial(pMaterial ? pMaterial : m_pDefaultMaterial.Get());
}

void CUI::SetClipRect(int32 x0, int32 y0, int32 x1, int32 y1)
{
	m_Batch.SetClipRect(x0, y0, x1, y1);
}

void CUI::ResetClipRect()
{
	m_Batch.ResetClipRect();
}

CFontHandle CUI::LoadFont(
	const char* pName,
	const char* pAtlasFilename,
	const char* pMetricsFilename)
{
	return m_pTextRenderer
		? m_pTextRenderer->LoadFont(pName, pAtlasFilename, pMetricsFilename)
		: s_InvalidFontHandle;
}

bool CUI::SetFont(CFontHandle hFont)
{
	return m_pTextRenderer ? m_pTextRenderer->SetFont(hFont) : false;
}

bool CUI::SetFontByName(const char* pName)
{
	return m_pTextRenderer ? m_pTextRenderer->SetFontByName(pName) : false;
}

void CUI::SetDefaultFont(CFontHandle hFont)
{
	if (m_pTextRenderer)
	{
		m_pTextRenderer->SetDefaultFont(hFont);
		m_pTextRenderer->ResetFont();
	}
}

CFontHandle CUI::GetCurrentFont() const
{
	return m_pTextRenderer
		? m_pTextRenderer->GetCurrentFont() : s_InvalidFontHandle;
}

void CUI::SetFontSize(float32 size)
{
	if (m_pTextRenderer)
	{
		m_pTextRenderer->SetFontSize(size);
	}
}

float32 CUI::GetFontSize() const
{
	return m_pTextRenderer ? m_pTextRenderer->GetFontSize() : 0.0f;
}

void CUI::SetTextSoftness(float32 softness)
{
	if (m_pTextRenderer)
	{
		m_pTextRenderer->SetTextSoftness(softness);
	}
}

float32 CUI::GetTextSoftness() const
{
	return m_pTextRenderer
		? m_pTextRenderer->GetTextSoftness() : 1.0f;
}

void CUI::SetLetterSpacing(float32 spacing)
{
	if (m_pTextRenderer)
	{
		m_pTextRenderer->SetLetterSpacing(spacing);
	}
}

float32 CUI::GetLetterSpacing() const
{
	return m_pTextRenderer ? m_pTextRenderer->GetLetterSpacing() : 0.0f;
}

void CUI::SetLineSpacing(float32 spacing)
{
	if (m_pTextRenderer)
	{
		m_pTextRenderer->SetLineSpacing(spacing);
	}
}

float32 CUI::GetLineSpacing() const
{
	return m_pTextRenderer ? m_pTextRenderer->GetLineSpacing() : 0.0f;
}

float32 CUI::MeasureTextWidth(const char* pText) const
{
	return m_pTextRenderer ? m_pTextRenderer->MeasureTextWidth(pText) : 0.0f;
}

float32 CUI::MeasureTextHeight(const char* pText) const
{
	return m_pTextRenderer ? m_pTextRenderer->MeasureTextHeight(pText) : 0.0f;
}

void CUI::DrawFilledRect(int32 x0, int32 y0, int32 x1, int32 y1)
{
	uint32 packedColor = CColor::PackColor(m_Batch.GetDrawColor());

	UnlitVertex_t vertices[4];
	for (int32 i = 0; i < 4; i++)
	{
		vertices[i].m_Color = packedColor;
		vertices[i].m_Z = 0.0f;
	}

	vertices[0].m_X = float32(x0);
	vertices[0].m_Y = float32(y0);
	vertices[0].m_U = 0.0f;
	vertices[0].m_V = 0.0f;

	vertices[1].m_X = float32(x1);
	vertices[1].m_Y = float32(y0);
	vertices[1].m_U = 1.0f;
	vertices[1].m_V = 0.0f;

	vertices[2].m_X = float32(x1);
	vertices[2].m_Y = float32(y1);
	vertices[2].m_U = 1.0f;
	vertices[2].m_V = 1.0f;

	vertices[3].m_X = float32(x0);
	vertices[3].m_Y = float32(y1);
	vertices[3].m_U = 0.0f;
	vertices[3].m_V = 1.0f;

	m_Batch.AddShape(vertices);
}

void CUI::DrawOutlinedRect(int32 x0, int32 y0, int32 x1, int32 y1)
{
	DrawFilledRect(x0, y0, x1, y0 + 2);
	DrawFilledRect(x0, y1 - 2, x1, y1);
	DrawFilledRect(x0, y0, x0 + 2, y1);
	DrawFilledRect(x1 - 2, y0, x1, y1);
}

void CUI::DrawLine(int32 x0, int32 y0, int32 x1, int32 y1)
{
	float32 dx = float32(x1 - x0);
	float32 dy = float32(y1 - y0);
	float32 len = CMaths::Sqrt(dx * dx + dy * dy);
	if (len < 0.001f)
	{
		return;
	}

	// Expands the line into a quad offset half a pixel perpendicular,
	// giving a 1px line
	float32 nx = -dy / len * 0.5f;
	float32 ny = dx / len * 0.5f;

	uint32 packedColor = CColor::PackColor(m_Batch.GetDrawColor());

	UnlitVertex_t vertices[4];
	for (int32 i = 0; i < 4; i++)
	{
		vertices[i].m_Color = packedColor;
		vertices[i].m_Z = 0.0f;
		vertices[i].m_U = 0.0f;
		vertices[i].m_V = 0.0f;
	}

	vertices[0].m_X = float32(x0) + nx;
	vertices[0].m_Y = float32(y0) + ny;

	vertices[1].m_X = float32(x0) - nx;
	vertices[1].m_Y = float32(y0) - ny;

	vertices[2].m_X = float32(x1) - nx;
	vertices[2].m_Y = float32(y1) - ny;

	vertices[3].m_X = float32(x1) + nx;
	vertices[3].m_Y = float32(y1) + ny;

	m_Batch.AddShape(vertices);
}

void CUI::DrawText(const char* pText, float32 x, float32 y)
{
	if (m_pTextRenderer && pText)
	{
		m_Batch.AddText();
		m_pTextRenderer->DrawText(
			pText,
			x,
			y,
			m_Batch.GetDrawColor(),
			CColor::Black,
			0.0f,
			m_pTextRenderer->GetTextSoftness());
	}
}

void CUI::DrawTextOutline(
	const char* pText,
	float32 x,
	float32 y,
	const CColor& outlineColor,
	float32 outlineWidth)
{
	if (m_pTextRenderer && pText)
	{
		m_Batch.AddText();
		m_pTextRenderer->DrawText(
			pText,
			x,
			y,
			m_Batch.GetDrawColor(),
			outlineColor,
			outlineWidth,
			m_pTextRenderer->GetTextSoftness());
	}
}

void CUI::DrawTextAnchored(
	const char* pText,
	Anchor_t::Enum anchor,
	float32 x,
	float32 y)
{
	if (!m_pTextRenderer || !pText)
	{
		return;
	}

	AnchoredPosition_t position = GetAnchoredPosition(pText, anchor, x, y);

	DrawText(pText, position.m_X, position.m_Y);
}

void CUI::DrawTextAnchoredOutline(
	const char* pText,
	Anchor_t::Enum anchor,
	float32 x,
	float32 y,
	const CColor& outlineColor,
	float32 outlineWidth)
{
	if (!m_pTextRenderer || !pText)
	{
		return;
	}

	AnchoredPosition_t position = GetAnchoredPosition(pText, anchor, x, y);

	DrawTextOutline(
		pText,
		position.m_X,
		position.m_Y,
		outlineColor,
		outlineWidth);
}

void CUI::BeginDraw()
{
	if (!m_pRenderer)
	{
		return;
	}

	Viewport_t viewport = m_pRenderer->GetViewport();
	CMatrix4 projection = CMaths::Orthographic(
		0.0f,
		float32(viewport.m_Width),
		0.0f,
		float32(viewport.m_Height),
		-1.0f,
		1.0f);

	m_Batch.BeginFrame(projection);

	if (m_pTextRenderer)
	{
		m_pTextRenderer->ResetFont();
		m_pTextRenderer->BeginTextBatch(projection);
	}
}

void CUI::EndDraw()
{
	m_Batch.Flush();

	if (m_pRenderer)
	{
		m_pRenderer->SetScissor(Rect_t(
			0,
			0,
			m_Batch.GetViewportWidth(),
			m_Batch.GetViewportHeight()));
	}
}

AnchoredPosition_t CUI::GetAnchoredPosition(
	const char* pText,
	Anchor_t::Enum anchor,
	float32 x,
	float32 y) const
{
	float32 textWidth = 0.0f;
	float32 ascent = 0.0f;
	float32 descent = 0.0f;
	float32 drop = 0.0f;
	if (m_pTextRenderer)
	{
		textWidth = m_pTextRenderer->MeasureTextWidth(pText);
		ascent = m_pTextRenderer->GetTextAscent();
		descent = m_pTextRenderer->GetTextDescent();
		drop = m_pTextRenderer->MeasureTextHeight(pText) -
			m_pTextRenderer->GetTextLineHeight();
	}

	float32 width = float32(m_Batch.GetViewportWidth());
	float32 height = float32(m_Batch.GetViewportHeight());

	float32 leftX = x;
	float32 centerX = width * 0.5f + x - textWidth * 0.5f;
	float32 rightX = width - x - textWidth;

	float32 bottomY = y + descent + drop;
	float32 centerY = height * 0.5f + y - (ascent - descent - drop) * 0.5f;
	float32 topY = height - y - ascent;

	AnchoredPosition_t position;

	switch (anchor)
	{
	case Anchor_t::TopLeft:
		position.m_X = leftX;
		position.m_Y = topY;

		break;
	case Anchor_t::Top:
		position.m_X = centerX;
		position.m_Y = topY;

		break;
	case Anchor_t::TopRight:
		position.m_X = rightX;
		position.m_Y = topY;

		break;
	case Anchor_t::Left:
		position.m_X = leftX;
		position.m_Y = centerY;

		break;
	case Anchor_t::Center:
		position.m_X = centerX;
		position.m_Y = centerY;

		break;
	case Anchor_t::Right:
		position.m_X = rightX;
		position.m_Y = centerY;

		break;
	case Anchor_t::BottomLeft:
		position.m_X = leftX;
		position.m_Y = bottomY;

		break;
	case Anchor_t::Bottom:
		position.m_X = centerX;
		position.m_Y = bottomY;

		break;
	case Anchor_t::BottomRight:
		position.m_X = rightX;
		position.m_Y = bottomY;

		break;
	default:
		position.m_X = leftX;
		position.m_Y = bottomY;

		break;
	}

	return position;
}
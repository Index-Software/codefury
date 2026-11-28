//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef UI_H
#define UI_H

#pragma once

#include "renderer/Renderer.h"
#include "renderer/text/Font.h"
#include "renderer/ui/UIBatch.h"
#include "utils/RefCounted.h"
#include "WrenBind.h"

class CTextRenderer;
class CMaterial;
class CMatrix4;
class CShader;

struct Anchor_t
{
	enum Enum
	{
		TopLeft,
		Top,
		TopRight,
		Left,
		Center,
		Right,
		BottomLeft,
		Bottom,
		BottomRight
	};
};

struct AnchoredPosition_t
{
	float32 m_X;
	float32 m_Y;
};

class CUI : public CWrenBindSingleton<CUI>
{
	WREN_SINGLETON_DECL()
public:
	CUI();
	~CUI();

	bool Init(IRenderer& renderer);
	void Shutdown();

	Rect_t GetViewport() const;
	uint32 GetViewportWidth() const
		WREN_BIND_SINGLETON("getViewportWidth()", s_pGUI);
	uint32 GetViewportHeight() const
		WREN_BIND_SINGLETON("getViewportHeight()", s_pGUI);
	uint32 GetScreenWidth() const
		WREN_BIND_SINGLETON("getScreenWidth()", s_pGUI);
	uint32 GetScreenHeight() const
		WREN_BIND_SINGLETON("getScreenHeight()", s_pGUI);

	void SetReferenceResolution(
		float32 width,
		float32 height)
		WREN_BIND_SINGLETON("setReferenceResolution(_,_)", s_pGUI);
	void ClearReferenceResolution()
		WREN_BIND_SINGLETON("clearReferenceResolution()", s_pGUI);

	void SetDrawColor(const CColor& color)
		WREN_BIND_SINGLETON("setDrawColor(_)", s_pGUI);
	CColor GetDrawColor() const WREN_BIND_SINGLETON("getDrawColor()", s_pGUI);
	void SetMaterial(CMaterial* pMaterial)
		WREN_BIND_SINGLETON("setMaterial(_)", s_pGUI);

	void SetClipRect(
		int32 x0,
		int32 y0,
		int32 x1,
		int32 y1) WREN_BIND_SINGLETON("setClipRect(_,_,_,_)", s_pGUI);
	void ResetClipRect() WREN_BIND_SINGLETON("resetClipRect()", s_pGUI);

	CFontHandle LoadFont(
		const char* pName,
		const char* pAtlasFilename,
		const char* pMetricsFilename)
		WREN_BIND_SINGLETON("loadFont(_,_,_)", s_pGUI);
	bool SetFont(CFontHandle hFont) WREN_BIND_SINGLETON("setFont(_)", s_pGUI);
	bool SetFontByName(
		const char* pName) WREN_BIND_SINGLETON("setFontByName(_)", s_pGUI);
	void SetDefaultFont(
		CFontHandle hFont) WREN_BIND_SINGLETON("setDefaultFont(_)", s_pGUI);
	CFontHandle GetCurrentFont() const
		WREN_BIND_SINGLETON("getCurrentFont()", s_pGUI);

	void SetFontSize(
		float32 size) WREN_BIND_SINGLETON("setFontSize(_)", s_pGUI);
	float32 GetFontSize() const WREN_BIND_SINGLETON("getFontSize()", s_pGUI);

	void SetTextSoftness(
		float32 softness) WREN_BIND_SINGLETON("setSoftness(_)", s_pGUI);
	float32 GetTextSoftness() const
		WREN_BIND_SINGLETON("getSoftness()", s_pGUI);
	void SetLetterSpacing(
		float32 spacing) WREN_BIND_SINGLETON("setLetterSpacing(_)", s_pGUI);
	float32 GetLetterSpacing() const
		WREN_BIND_SINGLETON("getLetterSpacing()", s_pGUI);
	void SetLineSpacing(
		float32 spacing) WREN_BIND_SINGLETON("setLineSpacing(_)", s_pGUI);
	float32 GetLineSpacing() const
		WREN_BIND_SINGLETON("getLineSpacing()", s_pGUI);

	float32 MeasureTextWidth(
		const char* pText) const
		WREN_BIND_SINGLETON("measureTextWidth(_)", s_pGUI);
	float32 MeasureTextHeight(
		const char* pText) const
		WREN_BIND_SINGLETON("measureTextHeight(_)", s_pGUI);

	void DrawFilledRect(int32 x0, int32 y0, int32 x1, int32 y1)
		WREN_BIND_SINGLETON("drawFilledRect(_,_,_,_)", s_pGUI);
	void DrawOutlinedRect(int32 x0, int32 y0, int32 x1, int32 y1)
		WREN_BIND_SINGLETON("drawOutlinedRect(_,_,_,_)", s_pGUI);
	void DrawLine(int32 x0, int32 y0, int32 x1, int32 y1)
		WREN_BIND_SINGLETON("drawLine(_,_,_,_)", s_pGUI);

	void DrawText(const char* pText, float32 x, float32 y)
		WREN_BIND_SINGLETON("drawText(_,_,_)", s_pGUI);
	void DrawTextOutline(
		const char* pText,
		float32 x,
		float32 y,
		const CColor& outlineColor,
		float32 outlineWidth)
		WREN_BIND_SINGLETON("drawTextOutline(_,_,_,_,_)", s_pGUI);
	void DrawTextAnchored(
		const char* pText,
		Anchor_t::Enum anchor,
		float32 x,
		float32 y) WREN_BIND_SINGLETON("drawTextAnchored(_,_,_,_)", s_pGUI);
	void DrawTextAnchoredOutline(
		const char* pText,
		Anchor_t::Enum anchor,
		float32 x,
		float32 y,
		const CColor& outlineColor,
		float32 outlineWidth)
		WREN_BIND_SINGLETON("drawTextAnchoredOutline(_,_,_,_,_,_)", s_pGUI);

	void BeginDraw() WREN_BIND_SINGLETON("beginDraw()", s_pGUI);
	void EndDraw() WREN_BIND_SINGLETON("endDraw()", s_pGUI);
private:
	AnchoredPosition_t GetAnchoredPosition(
		const char* pText,
		Anchor_t::Enum anchor,
		float32 x,
		float32 y) const;
	void BeginBatch(const CMatrix4& projection);

	CUIBatch m_Batch;
	IRenderer* m_pRenderer;
	CTextRenderer* m_pTextRenderer;
	CRef<CMaterial> m_pDefaultMaterial;
	CRef<CShader> m_pUnlitShader;
	float32 m_ReferenceWidth;
	float32 m_ReferenceHeight;
	float32 m_Width;
	float32 m_Height;
	float32 m_Scale;
	float32 m_OffsetX;
	float32 m_OffsetY;
};

#endif // UI_H
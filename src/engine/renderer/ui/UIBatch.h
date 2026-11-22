//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef UIBATCH_H
#define UIBATCH_H

#pragma once

#include "renderer/BatchPool.h"
#include "renderer/Renderer.h"
#include "renderer/text/Font.h"

class CTextRenderer;
class CMaterial;

struct UnlitVertex_t
{
	float32 m_X;
	float32 m_Y;
	float32 m_Z;
	uint32 m_Color;
	float32 m_U;
	float32 m_V;
};

struct UICommandType_t
{
	enum Enum
	{
		Shapes,
		Text
	};
};

struct UICommand_t
{
	Rect_t m_ClipRect;
	CMaterial* m_pMaterial;
	CFontHandle m_hFont;
	int32 m_VertexStart;
	int32 m_TextVertexStart;
	UICommandType_t::Enum m_Type;
	bool m_IsClipActive;
};

class CUIBatch
{
public:
	CUIBatch();

	bool Init(
		IRenderer& renderer,
		CTextRenderer* pTextRenderer,
		CMaterial* pDefaultMaterial);
	void Shutdown();

	void BeginFrame(const CMatrix4& projection);
	void SetDrawColor(const CColor& color);
	const CColor& GetDrawColor() const;
	void SetMaterial(CMaterial* pMaterial);
	void SetClipRect(int32 x0, int32 y0, int32 x1, int32 y1);
	void ResetClipRect();

	uint32 GetViewportWidth() const
	{
		return m_ViewportWidth;
	}

	uint32 GetViewportHeight() const
	{
		return m_ViewportHeight;
	}

	void AddShape(const UnlitVertex_t* pVertices);
	void AddText();
	void Flush();
private:
	void AddCommand(UICommandType_t::Enum type);
	void RenderShapes(const UICommand_t& command, int32 endVertex);

	CBatchPool m_BatchPool;
	CVertexLayout m_UnlitLayout;
	CUtlVector<UnlitVertex_t> m_UnlitVertices;
	CUtlVector<UICommand_t> m_Commands;
	Rect_t m_ClipRect;
	CColor m_DrawColor;
	IRenderer* m_pRenderer;
	CTextRenderer* m_pTextRenderer;
	CMaterial* m_pDefaultMaterial;
	CMaterial* m_pCurrentMaterial;
	UniformBlockLayoutHandle m_hUnlitUniformLayout;
	BufferHandle m_hUnlitUniformBuffer;
	uint32 m_ViewportWidth;
	uint32 m_ViewportHeight;
	bool m_IsClipActive;
};

#endif // UIBATCH_H
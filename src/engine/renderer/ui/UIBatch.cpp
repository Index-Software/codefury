//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/ui/UIBatch.h"
#include "renderer/material/Material.h"
#include "renderer/shader/Shader.h"
#include "renderer/shader/ShaderTypes.h"
#include "renderer/text/TextRenderer.h"
#include "dbg.h"

static const uint32 s_BatchVertexCapacity = 1024;
static const uint32 s_InitialBatchSlots = 16;

CUIBatch::CUIBatch() :
	m_DrawColor(255, 255, 255, 255),
	m_pRenderer(CF_NULL),
	m_pTextRenderer(CF_NULL),
	m_pDefaultMaterial(CF_NULL),
	m_pCurrentMaterial(CF_NULL),
	m_hUnlitUniformLayout(0),
	m_hUnlitUniformBuffer(0),
	m_ViewportWidth(1280),
	m_ViewportHeight(720),
	m_IsClipActive(false)
{
	m_UnlitLayout.AddAttribute(
		"position",
		static_cast<uint32>(VertexFormat_t::Float3),
		0,
		VertexSemantic_t::Position);
	m_UnlitLayout.AddAttribute(
		"color",
		static_cast<uint32>(VertexFormat_t::UByte4_Norm),
		3 * sizeof(float32),
		VertexSemantic_t::Color0);
	m_UnlitLayout.AddAttribute(
		"texCoord",
		static_cast<uint32>(VertexFormat_t::Float2),
		3 * sizeof(float32) + sizeof(uint32),
		VertexSemantic_t::TexCoord0);
	m_UnlitLayout.SetStride(sizeof(UnlitVertex_t));
}

bool CUIBatch::Init(
	IRenderer& renderer,
	CTextRenderer* pTextRenderer,
	CMaterial* pDefaultMaterial)
{
	m_pRenderer = &renderer;
	m_pTextRenderer = pTextRenderer;
	m_pDefaultMaterial = pDefaultMaterial;
	m_pCurrentMaterial = pDefaultMaterial;

	Viewport_t viewport = renderer.GetViewport();
	m_ViewportWidth = uint32(viewport.m_Width);
	m_ViewportHeight = uint32(viewport.m_Height);

	m_UnlitVertices.EnsureCapacity(s_BatchVertexCapacity * 4);

	if (!m_BatchPool.Init(
		m_pRenderer,
		s_BatchVertexCapacity * sizeof(UnlitVertex_t),
		s_BatchVertexCapacity / 4,
		s_InitialBatchSlots,
		m_pRenderer->GetMaxFrameLatency()))
	{
		Error("[CUIBatch::Init] Failed to create the UI batch buffers\n");

		return false;
	}

	UniformBlockLayout_t uniformLayout = CShaderLayouts::GetModelLayout();
	m_hUnlitUniformLayout = renderer.CreateUniformBlockLayout(uniformLayout);
	m_hUnlitUniformBuffer = renderer.CreateConstantBuffer(
		uniformLayout.m_Size,
		BufferUsage_t::Dynamic);

	if (m_hUnlitUniformLayout == 0 || m_hUnlitUniformBuffer == 0)
	{
		Error("[CUIBatch::Init] Failed to create the UI uniform resources\n");
		Shutdown();

		return false;
	}

	return true;
}

void CUIBatch::Shutdown()
{
	m_pCurrentMaterial = CF_NULL;
	m_pDefaultMaterial = CF_NULL;
	m_pTextRenderer = CF_NULL;

	m_BatchPool.Shutdown();

	if (m_hUnlitUniformBuffer)
	{
		m_pRenderer->DestroyBuffer(m_hUnlitUniformBuffer);
		m_hUnlitUniformBuffer = CF_NULL;
	}

	if (m_hUnlitUniformLayout)
	{
		m_pRenderer->DestroyUniformBlockLayout(m_hUnlitUniformLayout);
		m_hUnlitUniformLayout = 0;
	}

	m_pRenderer = CF_NULL;
}

void CUIBatch::BeginFrame(const CMatrix4& projection)
{
	m_Commands.RemoveAll();
	m_DrawColor = CColor::White;
	m_IsClipActive = false;
	m_pCurrentMaterial = m_pDefaultMaterial;

	Viewport_t viewport = m_pRenderer->GetViewport();
	m_ViewportWidth = uint32(viewport.m_Width);
	m_ViewportHeight = uint32(viewport.m_Height);

	CfModel_t modelData;
	modelData.m_MVP = projection;
	modelData.m_Model = CMatrix4(1.0f);

	m_pRenderer->UpdateBuffer(
		m_hUnlitUniformBuffer,
		&modelData,
		sizeof(CfModel_t),
		0);
	m_pRenderer->SetConstantBuffer(
		m_hUnlitUniformBuffer,
		m_hUnlitUniformLayout,
		CF_BINDING_MODEL,
		ShaderStageVertex);

	DepthStencilState_t depthState;
	depthState.m_IsDepthTest = false;
	depthState.m_IsDepthWrite = false;
	m_pRenderer->SetDepthStencilState(depthState);
}

void CUIBatch::SetDrawColor(const CColor& color)
{
	m_DrawColor = color;
}

const CColor& CUIBatch::GetDrawColor() const
{
	return m_DrawColor;
}

void CUIBatch::SetMaterial(CMaterial* pMaterial)
{
	m_pCurrentMaterial = pMaterial ? pMaterial : m_pDefaultMaterial;
}

void CUIBatch::SetClipRect(int32 x0, int32 y0, int32 x1, int32 y1)
{
	if (x1 <= x0 || y1 <= y0)
	{
		Warning("[CUIBatch] Invalid clip rect, ignoring\n");

		return;
	}

	m_ClipRect = Rect_t(
		x0,
		y0,
		static_cast<uint32>(x1 - x0),
		static_cast<uint32>(y1 - y0));
	m_IsClipActive = true;
}

void CUIBatch::ResetClipRect()
{
	m_IsClipActive = false;
}

void CUIBatch::AddShape(const UnlitVertex_t* pVertices)
{
	AddCommand(UICommandType_t::Shapes);

	for (int32 i = 0; i < 4; i++)
	{
		m_UnlitVertices.AddToTail(pVertices[i]);
	}
}

void CUIBatch::AddText()
{
	AddCommand(UICommandType_t::Text);
}

void CUIBatch::Flush()
{
	if (!m_pRenderer)
	{
		return;
	}

	int32 i = 0;
	while (i < m_Commands.Count())
	{
		const UICommand_t& command = m_Commands[i];

		int32 runEnd = i + 1;
		while (runEnd < m_Commands.Count() &&
			m_Commands[runEnd].m_Type == command.m_Type &&
			m_Commands[runEnd].m_pMaterial == command.m_pMaterial &&
			m_Commands[runEnd].m_IsClipActive == command.m_IsClipActive &&
			m_Commands[runEnd].m_hFont == command.m_hFont &&
			(!command.m_IsClipActive ||
				(m_Commands[runEnd].m_ClipRect.m_X == command.m_ClipRect.m_X &&
					m_Commands[runEnd].m_ClipRect.m_Y ==
					command.m_ClipRect.m_Y &&
					m_Commands[runEnd].m_ClipRect.m_Width ==
					command.m_ClipRect.m_Width &&
					m_Commands[runEnd].m_ClipRect.m_Height ==
					command.m_ClipRect.m_Height)))
		{
			runEnd++;
		}

		m_pRenderer->SetScissor(command.m_IsClipActive
			? command.m_ClipRect
			: Rect_t(0, 0, m_ViewportWidth, m_ViewportHeight));

		if (command.m_Type == UICommandType_t::Shapes)
		{
			int32 endVertex = runEnd < m_Commands.Count()
				? m_Commands[runEnd].m_VertexStart
				: m_UnlitVertices.Count();

			RenderShapes(command, endVertex);
		}
		else if (m_pTextRenderer)
		{
			int32 endTextVertex = runEnd < m_Commands.Count()
				? m_Commands[runEnd].m_TextVertexStart
				: m_pTextRenderer->GetVertexCount();

			m_pTextRenderer->RenderGlyphs(
				command.m_TextVertexStart,
				endTextVertex,
				command.m_hFont);
		}

		i = runEnd;
	}

	m_Commands.RemoveAll();
	m_UnlitVertices.RemoveAll();
}

void CUIBatch::AddCommand(UICommandType_t::Enum type)
{
	UICommand_t command;
	command.m_Type = type;
	command.m_pMaterial = m_pCurrentMaterial;
	command.m_hFont = m_pTextRenderer
		? m_pTextRenderer->GetCurrentFont() : s_InvalidFontHandle;
	command.m_ClipRect = m_ClipRect;
	command.m_IsClipActive = m_IsClipActive;
	command.m_VertexStart = m_UnlitVertices.Count();
	command.m_TextVertexStart = m_pTextRenderer
		? m_pTextRenderer->GetVertexCount() : 0;
	m_Commands.AddToTail(command);
}

void CUIBatch::RenderShapes(const UICommand_t& command, int32 endVertex)
{
	CMaterial* pMaterial = command.m_pMaterial
		? command.m_pMaterial : m_pDefaultMaterial;
	if (!pMaterial)
	{
		return;
	}

	if (pMaterial->GetShader())
	{
		m_pRenderer->SetShaderProgram(pMaterial->GetShader()->GetHandle());
	}

	if (pMaterial->GetTexture())
	{
		m_pRenderer->SetTexture(
			pMaterial->GetTexture()->GetHandle(),
			0,
			ShaderStageFragment,
			pMaterial->GetTexture()->GetWrapMode());
	}

	pMaterial->FlushProperties();

	m_pRenderer->SetBlendState(pMaterial->GetBlendState());

	m_pRenderer->SetConstantBuffer(
		m_hUnlitUniformBuffer,
		m_hUnlitUniformLayout,
		CF_BINDING_MODEL,
		ShaderStageVertex);

	int32 startQuad = command.m_VertexStart / 4;
	int32 endQuad = endVertex / 4;
	int32 quadsPerSlot = int32(s_BatchVertexCapacity) / 4;

	while (startQuad < endQuad)
	{
		BufferHandle hVertexBuffer = 0;
		if (!m_BatchPool.GetBatch(hVertexBuffer))
		{
			Error("[CUIBatch] Failed to get a batch buffer\n");

			return;
		}

		int32 chunkQuads = endQuad - startQuad;
		if (chunkQuads > quadsPerSlot)
		{
			chunkQuads = quadsPerSlot;
		}

		int32 chunkEnd = startQuad + chunkQuads;

		m_pRenderer->UpdateBuffer(
			hVertexBuffer,
			&m_UnlitVertices[startQuad * 4],
			chunkQuads * 4 * sizeof(UnlitVertex_t),
			0);

		m_pRenderer->SetVertexBuffer(
			hVertexBuffer,
			0,
			sizeof(UnlitVertex_t),
			0,
			&m_UnlitLayout);
		m_pRenderer->SetIndexBuffer(m_BatchPool.GetQuadIndexBuffer(), 0);

		m_pRenderer->DrawIndexed(
			static_cast<uint32>(chunkQuads * 6),
			0,
			0);

		startQuad = chunkEnd;
	}
}
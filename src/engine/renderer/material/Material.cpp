//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Material.h"
#include "dbg.h"
#include "renderer/RenderQueue.h"
#include "renderer/texture/Texture.h"
#include "renderer/shader/Shader.h"
#include "mathsfury/Vector4.h"
#include "utils/UniqueID.h"

static void CreatePropertyBuffer(
	IRenderer* pRenderer,
	const CRef<CShader>& pShader,
	const char* pName,
	uint32 size,
	int32 propertyCount,
	MaterialProperty_t& outProp)
{
	int32 binding = pRenderer->GetUniformBlockBinding(
		pShader->GetHandle(),
		pName);
	outProp.m_Binding = (binding >= 0)
		? uint32(binding)
		: uint32(propertyCount + 4);

	UniformBlockLayout_t layout;
	layout.m_UniformNames.AddToTail(pName);
	layout.m_Binding = outProp.m_Binding;
	layout.m_Size = size;
	outProp.m_hLayout = pRenderer->CreateUniformBlockLayout(layout);
	outProp.m_hBuffer = pRenderer->CreateConstantBuffer(
		size,
		BufferUsage_t::Dynamic);
}

CMaterial::CMaterial(IRenderer* pRenderer)
	: m_pRenderer(pRenderer)
	, m_hMaterial(CUniqueID::Generate())
	, m_RenderQueue(RenderQueue_t::Opaque)
{
	m_BlendState.m_IsEnabled = true;
	m_BlendState.m_IsAdditive = false;
}

CMaterial::~CMaterial()
{
	for (int32 i = m_Properties.FirstInorder();
		m_Properties.IsValidIndex(i);
		i = m_Properties.NextInorder(i))
	{
		const MaterialProperty_t& prop = m_Properties.Element(i);
		if (prop.m_hBuffer)
		{
			m_pRenderer->DestroyBuffer(prop.m_hBuffer);
		}

		if (prop.m_hLayout)
		{
			m_pRenderer->DestroyUniformBlockLayout(prop.m_hLayout);
		}
	}
}

void CMaterial::SetShader(const char* pShaderName)
{
	CRef<CShader> pShader = CRef<CShader>::Create(m_pRenderer);
	if (!pShader.IsValid() || !pShader->CreateFromSource(pShaderName))
	{
		Error("[CMaterial] Failed to create shader %s\n", pShaderName);

		return;
	}

	m_pShader = pShader;
}

void CMaterial::SetShader(CShader* pShader)
{
	m_pShader.Reset(pShader);
}

CRef<CShader> CMaterial::GetShader() const
{
	return m_pShader;
}

void CMaterial::SetTexture(CTexture* pTexture)
{
	m_pTexture.Reset(pTexture);
}

CRef<CTexture> CMaterial::GetTexture() const
{
	return m_pTexture;
}

void CMaterial::SetRenderQueue(int32 queue)
{
	m_RenderQueue = queue;
}

int32 CMaterial::GetRenderQueue() const
{
	return m_RenderQueue;
}

bool CMaterial::GetAlphaBlending() const
{
	return m_RenderQueue > RenderQueue_t::Cutoff;
}

void CMaterial::SetBlendEnabled(bool isEnabled)
{
	m_BlendState.m_IsEnabled = isEnabled;
}

void CMaterial::SetAdditive(bool isAdditive)
{
	m_BlendState.m_IsAdditive = isAdditive;
}

const BlendState_t& CMaterial::GetBlendState() const
{
	return m_BlendState;
}

MaterialHandle CMaterial::GetHandle() const
{
	return m_hMaterial;
}

void CMaterial::SetFloat(const char* pName, float32 value)
{
	if (!m_pShader.IsValid())
	{
		Error("[CMaterial] SetFloat %s needs a shader\n", pName);

		return;
	}

	int32 index = m_Properties.Find(CFixedString(pName));
	if (index != m_Properties.InvalidIndex())
	{
		m_Properties.Element(index).m_Value.m_X = value;

		return;
	}

	MaterialProperty_t prop;
	prop.m_Type = MaterialProperty_t::Float;
	prop.m_Value = CVector4(value, 0.0f, 0.0f, 0.0f);
	CreatePropertyBuffer(
		m_pRenderer,
		m_pShader,
		pName,
		sizeof(float32),
		m_Properties.Count(),
		prop);

	m_Properties.Insert(CFixedString(pName), prop);
}

void CMaterial::SetVector(const char* pName, const CVector4& value)
{
	if (!m_pShader.IsValid())
	{
		Error("[CMaterial] SetVector %s needs a shader\n", pName);

		return;
	}

	int32 index = m_Properties.Find(CFixedString(pName));
	if (index != m_Properties.InvalidIndex())
	{
		m_Properties.Element(index).m_Value = value;

		return;
	}

	MaterialProperty_t prop;
	prop.m_Type = MaterialProperty_t::Vector;
	prop.m_Value = value;
	CreatePropertyBuffer(
		m_pRenderer,
		m_pShader,
		pName,
		sizeof(CVector4),
		m_Properties.Count(),
		prop);

	m_Properties.Insert(CFixedString(pName), prop);
}

void CMaterial::FlushProperties() const
{
	for (int32 i = m_Properties.FirstInorder();
		m_Properties.IsValidIndex(i);
		i = m_Properties.NextInorder(i))
	{
		const MaterialProperty_t& prop = m_Properties.Element(i);
		if (!prop.m_hBuffer || !prop.m_hLayout)
		{
			continue;
		}

		if (prop.m_Type == MaterialProperty_t::Float)
		{
			m_pRenderer->UpdateBuffer(
				prop.m_hBuffer,
				&prop.m_Value.m_X,
				sizeof(float32),
				0);
		}
		else
		{
			m_pRenderer->UpdateBuffer(
				prop.m_hBuffer,
				&prop.m_Value,
				sizeof(CVector4),
				0);
		}

		m_pRenderer->SetConstantBuffer(
			prop.m_hBuffer,
			prop.m_hLayout,
			prop.m_Binding,
			ShaderStageAll);
	}
}

static CRef<CMaterial> s_pDefaultMaterial;

CMaterial* GetDefaultMaterial(IRenderer* pRenderer)
{
	if (!s_pDefaultMaterial.IsValid())
	{
		CRef<CMaterial> material = CRef<CMaterial>::Create(pRenderer);
		if (!material.IsValid())
		{
			return CF_NULL;
		}

		material->SetShader("Error");
		material->SetRenderQueue(RenderQueue_t::Overlay);

		CRef<CTexture> texture = CRef<CTexture>::Create(pRenderer);
		if (texture.IsValid() &&
			texture->LoadFromFile("assets/error_material.png"))
		{
			material->SetTexture(texture.Get());
		}

		s_pDefaultMaterial = material;
	}

	return s_pDefaultMaterial.Get();
}

void ReleaseDefaultMaterial()
{
	s_pDefaultMaterial.Reset();
}
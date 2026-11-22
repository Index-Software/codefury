//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Shader.h"
#include "dbg.h"
#include "renderer/texture/Texture.h"

CShader::CShader(IRenderer* pRenderer)
	: m_pRenderer(pRenderer)
	, m_hProgram(0)
	, m_hAddPassProgram(0)
	, m_Type(ShaderType_t::Default)
{
}

bool CShader::LoadFromFile(const char* pVertexPath, const char* pFragmentPath)
{
	Error("[CShader::LoadFromFile] Not implemented\n");

	return false;
}

bool CShader::CreateFromSource(const char* pName)
{
	m_Name = pName;
	m_hProgram = m_pRenderer->CreateShaderProgram(pName);

	if (!m_hProgram)
	{
		Error("[CShader] Failed to create shader program %s\n", pName);

		return false;
	}

	CFixedString addPassName = m_Name + CFixedString("Add");
	m_hAddPassProgram = 0;
	if (m_pRenderer->HasShaderProgram(addPassName))
	{
		m_hAddPassProgram = m_pRenderer->CreateShaderProgram(addPassName);
		if (!m_hAddPassProgram)
		{
			Warning(
				"[CShader] Failed to create add pass variant %s\n",
				addPassName.Get());
		}
	}

	return true;
}

void CShader::AddTexture(CTexture* pTexture, uint32 slot)
{
	if (slot > static_cast<uint32>(m_Textures.Count()))
	{
		return;
	}

	m_Textures.Set(static_cast<int32>(slot), pTexture);
}

void CShader::RemoveTexture(uint32 slot)
{
	if (slot >= static_cast<uint32>(m_Textures.Count()))
	{
		return;
	}

	m_Textures.Set(static_cast<int32>(slot), CF_NULL);
}

CRef<CTexture> CShader::GetTexture(uint32 slot) const
{
	return CRef<CTexture>(m_Textures.Get(static_cast<int32>(slot)));
}
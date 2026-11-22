//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SHADER_H
#define SHADER_H

#pragma once

#include "renderer/Renderer.h"
#include "renderer/texture/Texture.h"
#include "WrenBind.h"
#include "utils/UtlVector.h"
#include "utils/FixedString.h"
#include "utils/RefCounted.h"

struct ShaderType_t
{
	enum Enum
	{
		Default = 0,
		Skybox
	};
};

class CShader : public CRefCounted
{
public:
	CShader(IRenderer* pRenderer) WREN_CONSTRUCTOR("init new()");

	bool LoadFromFile(
		const char* pVertexPath,
		const char* pFragmentPath) WREN_BIND("loadFromFile(_,_)");
	bool CreateFromSource(const char* pName) WREN_BIND("createFromSource(_)");

	void SetType(ShaderType_t::Enum type) WREN_BIND("setType(_)")
	{
		m_Type = type;
	}

	ShaderType_t::Enum GetType() const WREN_BIND("getType")
	{
		return m_Type;
	}

	ShaderProgramHandle GetHandle() const WREN_BIND("getHandle")
	{
		return m_hProgram;
	}

	ShaderProgramHandle GetAddPassHandle() const
	{
		return m_hAddPassProgram ? m_hAddPassProgram : m_hProgram;
	}

	const char* GetName() const WREN_BIND("getName")
	{
		return m_Name.Get();
	}

	void AddTexture(
		CTexture* pTexture,
		uint32 slot = 0) WREN_BIND("addTexture(_,_)");
	void RemoveTexture(uint32 slot) WREN_BIND("removeTexture(_)");
	CRef<CTexture> GetTexture(uint32 slot) const WREN_BIND("getTexture(_)");

	const CUtlVector<CTexture*>& GetTextures() const
	{
		return m_Textures.Items();
	}
private:
	CFixedString m_Name;
	CRefList<CTexture> m_Textures;

	IRenderer* m_pRenderer;
	ShaderProgramHandle m_hProgram;
	ShaderProgramHandle m_hAddPassProgram;
	ShaderType_t::Enum m_Type;
};

#endif // SHADER_H
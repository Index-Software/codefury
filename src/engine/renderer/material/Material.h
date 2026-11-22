//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef MATERIAL_H
#define MATERIAL_H

#pragma once

#include "renderer/Renderer.h"
#include "renderer/shader/Shader.h"
#include "WrenBind.h"
#include "renderer/texture/Texture.h"
#include "utils/UtlMap.h"
#include "utils/FixedString.h"
#include "utils/RefCounted.h"
#include "mathsfury/Vector4.h"

struct MaterialProperty_t
{
	enum Type_t
	{
		Float,
		Vector
	};

	CVector4 m_Value;
	uint32 m_hBuffer;
	uint32 m_hLayout;
	uint32 m_Binding;
	Type_t m_Type;

	MaterialProperty_t() :
		m_Value(0.0f, 0.0f, 0.0f, 0.0f),
		m_hBuffer(0),
		m_hLayout(0),
		m_Binding(0),
		m_Type(Float)
	{
	}
};

class CMaterial : public CRefCounted
{
public:
	CMaterial(IRenderer* pRenderer) WREN_CONSTRUCTOR("init new()");
	virtual ~CMaterial() CF_OVERRIDE;

	void SetShader(const char* pShaderName) WREN_BIND("setShaderName(_)");
	void SetShader(CShader* pShader) WREN_BIND("setShader(_)");
	CRef<CShader> GetShader() const WREN_BIND("getShader");
	void SetTexture(CTexture* pTexture) WREN_BIND("setTexture(_)");
	CRef<CTexture> GetTexture() const WREN_BIND("getTexture");
	void SetRenderQueue(int32 queue) WREN_BIND("setRenderQueue(_)");
	int32 GetRenderQueue() const WREN_BIND("getRenderQueue");
	bool GetAlphaBlending() const WREN_BIND("getAlphaBlending");
	void SetBlendEnabled(bool isEnabled) WREN_BIND("setBlendEnabled(_)");
	void SetAdditive(bool isAdditive) WREN_BIND("setAdditive(_)");
	const BlendState_t& GetBlendState() const;
	MaterialHandle GetHandle() const WREN_BIND("getHandle");

	void SetFloat(const char* pName, float32 value) WREN_BIND("setFloat(_,_)");
	void SetVector(
		const char* pName,
		const CVector4& value) WREN_BIND("setVector(_,_)");
	void FlushProperties() const WREN_BIND("flushProperties()");

	const CUtlMap<CFixedString, MaterialProperty_t>& GetProperties() const
		WREN_BIND("getProperties")
	{
		return m_Properties;
	}
private:
	CUtlMap<CFixedString, MaterialProperty_t> m_Properties;

	IRenderer* m_pRenderer;
	CRef<CShader> m_pShader;
	CRef<CTexture> m_pTexture;

	MaterialHandle m_hMaterial;
	int32 m_RenderQueue;
	BlendState_t m_BlendState;
};

CMaterial* GetDefaultMaterial(IRenderer* pRenderer);
void ReleaseDefaultMaterial();

#endif // MATERIAL_H
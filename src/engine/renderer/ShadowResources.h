//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SHADOWRESOURCES_H
#define SHADOWRESOURCES_H

#pragma once

#include "renderer/Renderer.h"
#include "utils/RefCounted.h"

class CMaterial;

class CShadowResources
{
public:
	CShadowResources();
	~CShadowResources();

	bool Init(IRenderer& renderer);
	void Shutdown();

	bool HasMainShadow() const
	{
		return m_hMainRenderTarget != 0 && m_pCastMaterial.IsValid();
	}

	bool HasPointShadow() const
	{
		return m_hPointRenderTarget != 0 && m_pCastMaterial.IsValid();
	}

	RenderTargetHandle GetMainRenderTarget() const
	{
		return m_hMainRenderTarget;
	}

	TextureHandle GetMainTexture() const
	{
		return m_hMainTexture;
	}

	RenderTargetHandle GetPointRenderTarget() const
	{
		return m_hPointRenderTarget;
	}

	TextureHandle GetPointTexture() const
	{
		return m_hPointTexture;
	}

	CMaterial* GetCastMaterial() const
	{
		return m_pCastMaterial.Get();
	}

	uint32 GetMainSize() const
	{
		return m_MainSize;
	}
private:
	CShadowResources(const CShadowResources&);
	CShadowResources& operator=(const CShadowResources&);

	CRef<CMaterial> m_pCastMaterial;
	IRenderer* m_pRenderer;
	RenderTargetHandle m_hMainRenderTarget;
	RenderTargetHandle m_hPointRenderTarget;
	TextureHandle m_hMainTexture;
	TextureHandle m_hPointTexture;
	uint32 m_MainSize;
};

#endif // SHADOWRESOURCES_H

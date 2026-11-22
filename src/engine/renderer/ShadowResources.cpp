//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/ShadowResources.h"
#include "renderer/ShadowLayout.h"
#include "renderer/shader/Shader.h"
#include "renderer/material/Material.h"
#include "config/ConfigVar.h"
#include "dbg.h"

static CConfigVar s_rShadowMapSize(
	"r_shadowmapsize",
	float32(s_MainShadowMapSize),
	ConfigVarArchive | ConfigVarNeedsApply,
	256.0f,
	float32(s_MainShadowMapSize));

static TextureHandle SetupCompareTexture(
	IRenderer& renderer,
	RenderTargetHandle hRenderTarget)
{
	if (!hRenderTarget)
	{
		return 0;
	}

	const TextureHandle hTexture =
		renderer.GetRenderTargetDepthTexture(hRenderTarget);
	if (hTexture)
	{
		renderer.SetTextureCompareMode(
			hTexture,
			TextureCompareMode_t::RefToTexture);
	}

	return hTexture;
}

CShadowResources::CShadowResources() :
	m_pRenderer(CF_NULL),
	m_hMainRenderTarget(0),
	m_hPointRenderTarget(0),
	m_hMainTexture(0),
	m_hPointTexture(0),
	m_MainSize(0)
{
}

CShadowResources::~CShadowResources()
{
	Shutdown();
}

bool CShadowResources::Init(IRenderer& renderer)
{
	Shutdown();

	m_pRenderer = &renderer;
	m_MainSize = uint32(s_rShadowMapSize.GetFloat());

	m_hMainRenderTarget = renderer.CreateRenderTarget(
		m_MainSize,
		m_MainSize,
		TextureFormat_t::RGBA8,
		TextureFormat_t::Depth24Stencil8);

#ifdef CF_POINT_SHADOW_ATLAS
	m_hPointRenderTarget = renderer.CreateRenderTarget(
		s_PointShadowAtlasWidth,
		s_PointShadowAtlasHeight,
		TextureFormat_t::RGBA8,
		TextureFormat_t::Depth24Stencil8);
#else // !CF_POINT_SHADOW_ATLAS
	m_hPointRenderTarget = renderer.CreateRenderTargetCube(
		s_PointShadowCubeSize,
		TextureFormat_t::RGBA8,
		TextureFormat_t::Depth24Stencil8);
#endif // !CF_POINT_SHADOW_ATLAS

	m_hMainTexture = SetupCompareTexture(renderer, m_hMainRenderTarget);
	m_hPointTexture = SetupCompareTexture(renderer, m_hPointRenderTarget);

	CRef<CShader> pDepthShader = CRef<CShader>::Create(&renderer);
	if (pDepthShader.IsValid() && pDepthShader->CreateFromSource("lighting/ShadowCast"))
	{
		m_pCastMaterial = CRef<CMaterial>::Create(&renderer);
		m_pCastMaterial->SetShader(pDepthShader.Get());
	}

	if (!HasMainShadow() || !HasPointShadow())
	{
		Error("[CShadowResources] Failed to create shadow resources\n");

		return false;
	}

	return true;
}

void CShadowResources::Shutdown()
{
	if (m_pRenderer)
	{
		if (m_hMainRenderTarget)
		{
			m_pRenderer->DestroyRenderTarget(m_hMainRenderTarget);
		}

		if (m_hPointRenderTarget)
		{
			m_pRenderer->DestroyRenderTarget(m_hPointRenderTarget);
		}

		m_pRenderer = CF_NULL;
	}

	m_hMainRenderTarget = 0;
	m_hPointRenderTarget = 0;
	m_hMainTexture = 0;
	m_hPointTexture = 0;
	m_pCastMaterial.Reset();
}
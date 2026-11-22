//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "ForwardBasePass.h"
#include "renderer/RenderQueue.h"
#include "shaders/ShaderBindings.h"
#include "renderer/RenderItem.h"
#include "ecs/components/Light.h"
#include "renderer/material/Material.h"
#include "renderer/texture/Texture.h"

static void RenderItems(
	const CUtlVector<const RenderItem_t*>& items,
	RenderPassContext_t& context,
	bool isBlendEnabled,
	TextureHandle hShadowMapTexture)
{
	BlendState_t blendStateConfig;
	blendStateConfig.m_IsEnabled = isBlendEnabled;
	context.m_pRenderer->SetBlendState(blendStateConfig);

	for (int32 i = 0; i < items.Count(); i++)
	{
		const RenderItem_t* pItem = items[i];
		CMaterial* pMaterial = ::GetMaterialForRenderItem(
			*pItem,
			*context.m_pRenderer);

		if (hShadowMapTexture != 0)
		{
			context.m_pRenderer->SetTexture(
				hShadowMapTexture,
				CF_BINDING_SHADOW_MAP,
				ShaderStageFragment,
				TextureWrapMode_t::ClampToEdge);
		}

		if (context.m_pGlobalEnvironmentMap)
		{
			context.m_pGlobalEnvironmentMap->Bind(CF_BINDING_GLOBAL_ENV_MAP);
		}

		context.m_pEnvironmentLighting->Bind();

		::DrawRenderItem(
			*pItem,
			*context.m_pRenderer,
			*context.m_pConstantBuffers,
			context.m_ViewProjection,
			pMaterial);
	}
}

void CForwardBasePass::Render(
	RenderPassContext_t& context,
	CRenderQueueManager* pManager)
{
	context.m_pRenderer->SetRenderTarget(0);

	const Light_t* pDirLight = CF_NULL;
	for (int32 i = 0; i < context.m_pLightContext->m_Count; i++)
	{
		if (context.m_pLightContext->m_Lights[i].m_Type ==
			LightType_t::Directional)
		{
			pDirLight = &context.m_pLightContext->m_Lights[i];

			break;
		}
	}

	LightContext_t baseLightContext;
	baseLightContext.m_Count = 0;
	if (pDirLight)
	{
		baseLightContext.m_Lights[0] = *pDirLight;
		baseLightContext.m_Count = 1;
	}

	pManager->UpdateGlobalBuffers(
		context.m_LightSpaceMatrix,
		baseLightContext,
		pDirLight,
		false,
		-1);

	TextureHandle hShadowMapTexture =
		context.m_pShadowResources->GetMainTexture();

	for (int32 i = 0; i < context.m_pQueues->Count(); i++)
	{
		RenderQueueEntry_t& queueEntry = (*context.m_pQueues)[i];
		CUtlVector<RenderItem_t>& items = queueEntry.m_Items;
		if (items.IsEmpty())
		{
			continue;
		}

		CUtlVector<const RenderItem_t*> opaqueItems;
		CUtlVector<const RenderItem_t*> transparentItems;
		int32 queueID = queueEntry.m_QueueID;
		::SetDepthStencilForQueue(*context.m_pRenderer, queueID);

		for (int32 j = 0; j < items.Count(); j++)
		{
			const RenderItem_t& renderItem = items[j];
			CMaterial* pMaterial = ::GetMaterialForRenderItem(
				renderItem,
				*context.m_pRenderer);
			if (pMaterial && pMaterial->GetAlphaBlending())
			{
				transparentItems.AddToTail(&renderItem);
			}
			else
			{
				opaqueItems.AddToTail(&renderItem);
			}
		}

		RenderItems(opaqueItems, context, false, hShadowMapTexture);
		RenderItems(transparentItems, context, true, hShadowMapTexture);
	}
}
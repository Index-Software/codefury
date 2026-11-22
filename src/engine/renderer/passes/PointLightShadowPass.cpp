//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "PointLightShadowPass.h"
#include "renderer/RenderQueue.h"
#include "renderer/RenderItem.h"
#include "renderer/ShadowLayout.h"
#include "ecs/components/Light.h"
#include "mathsfury/Vector3.h"
#include "mathsfury/Matrix4.h"
#include "mathsfury/Maths.h"
#include "config/ConfigVar.h"
#include "ecs/components/Transform.h"
#include <math.h>

static CConfigVar s_rShadowCache(
	"r_shadowcache",
	true,
	ConfigVarNone);

static const int32 s_ShadowCacheRedrawFrames = 60;

static bool IsShadowCasterQueue(int32 queueID)
{
	return queueID != RenderQueue_t::Background &&
		queueID < RenderQueue_t::Cutoff;
}

CPointLightShadowPass::CPointLightShadowPass() :
	m_LightPos(0.0f, 0.0f, 0.0f),
	m_hRenderTarget(0),
	m_RedrawFrames(0),
	m_IsCached(false)
{
}

void CPointLightShadowPass::Render(
	RenderPassContext_t& context,
	CRenderQueueManager* pManager)
{
	if (!context.m_pShadowResources->HasPointShadow())
	{
		return;
	}

	if (context.m_CurrentLightIndex < 0 ||
		context.m_CurrentLightIndex >= context.m_pLightContext->m_Count)
	{
		return;
	}

	const Light_t& pointLight =
		context.m_pLightContext->m_Lights[context.m_CurrentLightIndex];
	CVector3 lightPos = pointLight.m_Position;
	const Viewport_t originalViewport = context.m_pRenderer->GetViewport();

	if (UpdateCache(context, lightPos))
	{
		m_RedrawFrames = s_ShadowCacheRedrawFrames;
	}

	if (m_RedrawFrames == 0 && s_rShadowCache.GetBool())
	{
		return;
	}

	if (m_RedrawFrames > 0)
	{
		m_RedrawFrames--;
	}

#ifdef CF_POINT_SHADOW_ATLAS
	context.m_pRenderer->SetRenderTarget(
		context.m_pShadowResources->GetPointRenderTarget());

	context.m_pRenderer->Clear(
		ClearDepth | ClearColor,
		CColor(1.0f, 1.0f, 1.0f, 1.0f),
		1.0f,
		0);
#endif // CF_POINT_SHADOW_ATLAS

	CVector3 targets[6] = {
		CVector3(1.0f, 0.0f, 0.0f),
		CVector3(-1.0f, 0.0f, 0.0f),
		CVector3(0.0f, 1.0f, 0.0f),
		CVector3(0.0f, -1.0f, 0.0f),
		CVector3(0.0f, 0.0f, 1.0f),
		CVector3(0.0f, 0.0f, -1.0f)
	};

	CVector3 ups[6] = {
		CVector3(0.0f, -1.0f, 0.0f),
		CVector3(0.0f, -1.0f, 0.0f),
		CVector3(0.0f, 0.0f, 1.0f),
		CVector3(0.0f, 0.0f, -1.0f),
		CVector3(0.0f, -1.0f, 0.0f),
		CVector3(0.0f, -1.0f, 0.0f)
	};

#ifdef CF_POINT_SHADOW_ATLAS
	const float32 faceFOV =
		2.0f * atanf(1.0f / s_PointShadowTileScale) * CMaths::Rad2Deg;
#else // !CF_POINT_SHADOW_ATLAS
	const float32 faceFOV = 90.0f;
#endif // !CF_POINT_SHADOW_ATLAS
	CMatrix4 lightProj = CMaths::Perspective(
		faceFOV,
		1.0f,
		float32(CF_POINT_SHADOW_NEAR_PLANE),
		float32(CF_POINT_SHADOW_FAR_PLANE));

	for (int32 face = 0; face < 6; face++)
	{
		CMatrix4 lightView = CMaths::LookAt(
			lightPos,
			lightPos + targets[face],
			ups[face]);
		CMatrix4 faceLightSpaceMatrix = lightProj * lightView;

		pManager->UpdateGlobalBuffers(
			faceLightSpaceMatrix,
			*context.m_pLightContext,
			CF_NULL,
			false,
			context.m_CurrentLightIndex);

#ifdef CF_POINT_SHADOW_ATLAS
		const float32 tileSize = float32(s_PointShadowTileSize);
		const float32 tileX =
			float32(face % s_PointShadowAtlasColumns) * tileSize;
		const float32 tileY =
			float32(face / s_PointShadowAtlasColumns) * tileSize;
		context.m_pRenderer->SetViewport(
			Viewport_t(tileX, tileY, tileSize, tileSize));
		context.m_pRenderer->SetScissor(
			Rect_t(
				int32(tileX),
				int32(tileY),
				int32(tileSize),
				int32(tileSize)));
#else // !CF_POINT_SHADOW_ATLAS
		context.m_pRenderer->SetRenderTarget(
			context.m_pShadowResources->GetPointRenderTarget(),
			face);

		context.m_pRenderer->Clear(
			ClearDepth | ClearColor,
			CColor(1.0f, 1.0f, 1.0f, 1.0f),
			1.0f,
			0);
#endif // !CF_POINT_SHADOW_ATLAS

		for (int32 i = 0; i < context.m_pQueues->Count(); i++)
		{
			RenderQueueEntry_t& queueEntry = (*context.m_pQueues)[i];
			if (!IsShadowCasterQueue(queueEntry.m_QueueID))
			{
				continue;
			}

			::SetDepthStencilForQueue(
				*context.m_pRenderer,
				queueEntry.m_QueueID);
			BlendState_t blendStateConfig;
			blendStateConfig.m_IsEnabled = false;
			context.m_pRenderer->SetBlendState(blendStateConfig);

			for (int32 j = 0; j < queueEntry.m_Items.Count(); j++)
			{
				const RenderItem_t& renderItem = queueEntry.m_Items[j];
				::DrawRenderItem(
					renderItem,
					*context.m_pRenderer,
					*context.m_pConstantBuffers,
					faceLightSpaceMatrix,
					context.m_pShadowResources->GetCastMaterial());
			}
		}
	}

	context.m_pRenderer->SetViewport(originalViewport);
}

bool CPointLightShadowPass::UpdateCache(
	const RenderPassContext_t& context,
	const CVector3& lightPos)
{
	bool hasChanged = !m_IsCached ||
		m_hRenderTarget != context.m_pShadowResources->GetPointRenderTarget() ||
		m_LightPos != lightPos;

	m_IsCached = true;
	m_hRenderTarget = context.m_pShadowResources->GetPointRenderTarget();
	m_LightPos = lightPos;

	int32 casterCount = 0;
	for (int32 i = 0; i < context.m_pQueues->Count(); i++)
	{
		const RenderQueueEntry_t& queueEntry = (*context.m_pQueues)[i];
		if (!IsShadowCasterQueue(queueEntry.m_QueueID))
		{
			continue;
		}

		for (int32 j = 0; j < queueEntry.m_Items.Count(); j++)
		{
			const RenderItem_t& renderItem = queueEntry.m_Items[j];
			CMatrix4 model = renderItem.m_pTransform
				? renderItem.m_pTransform->GetMatrix()
				: CMatrix4(1.0f);

			if (casterCount == m_Casters.Count())
			{
				m_Casters.AddToTail(ShadowCaster_t());
			}

			ShadowCaster_t& caster = m_Casters[casterCount];
			if (caster.m_pMesh != renderItem.m_pMesh ||
				caster.m_SubMeshIndex != renderItem.m_SubMeshIndex ||
				caster.m_Model != model)
			{
				caster.m_Model = model;
				caster.m_pMesh = renderItem.m_pMesh;
				caster.m_SubMeshIndex = renderItem.m_SubMeshIndex;
				hasChanged = true;
			}

			casterCount++;
		}
	}

	if (casterCount != m_Casters.Count())
	{
		m_Casters.SetCount(casterCount);
		hasChanged = true;
	}

	return hasChanged;
}
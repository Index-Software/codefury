//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "MainLightShadowPass.h"
#include "renderer/RenderQueue.h"
#include "renderer/RenderItem.h"
#include "ecs/components/Camera.h"
#include "ecs/components/Light.h"
#include "mathsfury/Maths.h"

void CMainLightShadowPass::Render(
	RenderPassContext_t& context,
	CRenderQueueManager* pManager)
{
	if (!context.m_pShadowResources->HasMainShadow())
	{
		return;
	}

	int32 dirLightIndex = -1;
	for (int32 i = 0; i < context.m_pLightContext->m_Count; i++)
	{
		if (context.m_pLightContext->m_Lights[i].m_Type ==
			LightType_t::Directional && dirLightIndex == -1)
		{
			dirLightIndex = i;
		}
	}

	if (dirLightIndex != -1)
	{
		const Light_t& dirLight =
			context.m_pLightContext->m_Lights[dirLightIndex];

		CVector3 shadowCenter = CVector3(0.0f, 0.0f, 0.0f);
		if (context.m_pCamera)
		{
			CVector3 camPos = context.m_pCamera->GetPosition();
			CVector3 camForward = CMaths::Normalise(
				context.m_pCamera->GetTarget() - camPos);
			shadowCenter = camPos + camForward * dirLight.m_ShadowCenterOffset;
		}

		CVector3 lightTarget = shadowCenter;
		CVector3 lightPos = lightTarget -
			dirLight.m_Direction * (dirLight.m_ShadowDistance * 0.5f);
		CVector3 up = CVector3(0.0f, 1.0f, 0.0f);
		if (CMaths::Abs(dirLight.m_Direction.m_Y) > 0.999f)
		{
			up = CVector3(0.0f, 0.0f, 1.0f);
		}
		CMatrix4 lightView = CMaths::LookAt(lightPos, lightTarget, up);

		CMatrix4 lightProj = CMaths::Orthographic(
			-dirLight.m_ShadowOrthoSize,
			dirLight.m_ShadowOrthoSize,
			-dirLight.m_ShadowOrthoSize,
			dirLight.m_ShadowOrthoSize,
			dirLight.m_ShadowNearPlane,
			dirLight.m_ShadowFarPlane);
		context.m_LightSpaceMatrix = lightProj * lightView;

		pManager->UpdateGlobalBuffers(
			context.m_LightSpaceMatrix,
			*context.m_pLightContext,
			&dirLight,
			false,
			-1);

		context.m_pRenderer->SetRenderTarget(
			context.m_pShadowResources->GetMainRenderTarget());

		context.m_pRenderer->Clear(
			ClearDepth | ClearColor,
			CColor(1.0f, 1.0f, 1.0f, 1.0f),
			1.0f,
			0);

		for (int32 i = 0; i < context.m_pQueues->Count(); i++)
		{
			RenderQueueEntry_t& queueEntry = (*context.m_pQueues)[i];
			if (queueEntry.m_QueueID == RenderQueue_t::Background ||
				queueEntry.m_QueueID >= RenderQueue_t::Cutoff)
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
					context.m_LightSpaceMatrix,
					context.m_pShadowResources->GetCastMaterial());
			}
		}
	}
}
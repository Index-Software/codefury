//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "ForwardAddPass.h"
#include "renderer/RenderQueue.h"
#include "shaders/ShaderBindings.h"
#include "renderer/RenderItem.h"
#include "ecs/components/Light.h"
#include "ecs/components/Camera.h"
#include "mathsfury/Vector4.h"
#include "mathsfury/Matrix4.h"
#include "mathsfury/Maths.h"
#include <math.h>

static bool GetSphereAxisBounds(
	float32 axisCenter,
	float32 depth,
	float32 radius,
	float32 projectionScale,
	float32& outMin,
	float32& outMax)
{
	const float32 tangentSq =
		axisCenter * axisCenter + depth * depth - radius * radius;
	if (tangentSq <= 0.0f)
	{
		return false;
	}

	const float32 tangent = CMaths::Sqrt(tangentSq);
	outMin = 1.0f;
	outMax = -1.0f;
	for (int32 side = -1; side <= 1; side += 2)
	{
		const float32 sideRadius = float32(side) * radius;
		const float32 tangentDepth = depth * tangent + axisCenter * sideRadius;
		if (tangentDepth <= 0.0f)
		{
			return false;
		}

		const float32 bound = projectionScale *
			(axisCenter * tangent - depth * sideRadius) / tangentDepth;
		outMin = CMaths::Min(outMin, bound);
		outMax = CMaths::Max(outMax, bound);
	}

	return true;
}

static bool GetLightScissorRect(
	const CCamera& camera,
	const Light_t& light,
	uint32 width,
	uint32 height,
	Rect_t& outRect)
{
	const CMatrix4 projection = camera.GetProjectionMatrix();
	const CVector3& position = light.m_Position;
	const CVector4 viewCenter = camera.GetViewMatrix() *
		CVector4(position.m_X, position.m_Y, position.m_Z, 1.0f);
	const float32 depth = -viewCenter.m_Z;
	const float32 scaleX =
		(projection * CVector4(1.0f, 0.0f, 0.0f, 0.0f)).m_X;
	const float32 scaleY =
		(projection * CVector4(0.0f, 1.0f, 0.0f, 0.0f)).m_Y;

	float32 minX;
	float32 maxX;
	if (!GetSphereAxisBounds(
		viewCenter.m_X,
		depth,
		light.m_Radius,
		scaleX,
		minX,
		maxX))
	{
		minX = -1.0f;
		maxX = 1.0f;
	}

	float32 minY;
	float32 maxY;
	if (!GetSphereAxisBounds(
		viewCenter.m_Y,
		depth,
		light.m_Radius,
		scaleY,
		minY,
		maxY))
	{
		minY = -1.0f;
		maxY = 1.0f;
	}

	minX = CMaths::Clamp(minX, -1.0f, 1.0f);
	minY = CMaths::Clamp(minY, -1.0f, 1.0f);
	maxX = CMaths::Clamp(maxX, -1.0f, 1.0f);
	maxY = CMaths::Clamp(maxY, -1.0f, 1.0f);
	if (minX >= maxX || minY >= maxY)
	{
		return false;
	}

	const int32 x0 = int32(floorf((minX * 0.5f + 0.5f) * float32(width)));
	const int32 y0 = int32(floorf((0.5f - maxY * 0.5f) * float32(height)));
	const int32 x1 = int32(ceilf((maxX * 0.5f + 0.5f) * float32(width)));
	const int32 y1 = int32(ceilf((0.5f - minY * 0.5f) * float32(height)));
	outRect = Rect_t(x0, y0, uint32(x1 - x0), uint32(y1 - y0));

	return true;
}

static bool GetLightDepthBounds(
	const CMatrix4& viewProjection,
	const Light_t& light,
	float32& outMinDepth,
	float32& outMaxDepth)
{
	const CVector3& position = light.m_Position;
	const float32 radius = light.m_Radius;
	const CVector4 center(position.m_X, position.m_Y, position.m_Z, 1.0f);
	const float32 centerDepth = (viewProjection * center).m_W;
	if (centerDepth + radius <= 0.0f)
	{
		return false;
	}

	const float32 forwardX =
		(viewProjection * (center + CVector4(1.0f, 0.0f, 0.0f, 0.0f))).m_W -
		centerDepth;
	const float32 forwardY =
		(viewProjection * (center + CVector4(0.0f, 1.0f, 0.0f, 0.0f))).m_W -
		centerDepth;
	const float32 forwardZ =
		(viewProjection * (center + CVector4(0.0f, 0.0f, 1.0f, 0.0f))).m_W -
		centerDepth;
	const float32 scale = radius / CMaths::Sqrt(
		forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
	const CVector4 offset(
		forwardX * scale,
		forwardY * scale,
		forwardZ * scale,
		0.0f);

	outMinDepth = 0.0f;
	if (centerDepth - radius > 0.0f)
	{
		const CVector4 nearClip = viewProjection * (center - offset);
		outMinDepth = CMaths::Clamp(
			nearClip.m_Z / nearClip.m_W * 0.5f + 0.5f,
			0.0f,
			1.0f);
	}

	const CVector4 farClip = viewProjection * (center + offset);
	outMaxDepth = CMaths::Clamp(
		farClip.m_Z / farClip.m_W * 0.5f + 0.5f,
		0.0f,
		1.0f);

	return true;
}

void CForwardAddPass::Render(
	RenderPassContext_t& context,
	CRenderQueueManager* pManager)
{
	if (context.m_CurrentLightIndex < 0 ||
		context.m_CurrentLightIndex >= context.m_pLightContext->m_Count) return;

	const Light_t* pPointLight =
		&context.m_pLightContext->m_Lights[context.m_CurrentLightIndex];
	if (pPointLight->m_Type != LightType_t::Point)
	{
		return;
	}

	const Viewport_t originalViewport = context.m_pRenderer->GetViewport();
	context.m_pRenderer->SetRenderTarget(0);

	uint32 width;
	uint32 height;
	context.m_pRenderer->GetFramebufferSize(width, height);

	Rect_t scissorRect(0, 0, width, height);
	if (context.m_pCamera)
	{
		const bool isVisible = GetLightScissorRect(
			*context.m_pCamera,
			*pPointLight,
			width,
			height,
			scissorRect);
		if (!isVisible)
		{
			return;
		}
	}

	float32 minDepth;
	float32 maxDepth;
	const bool isInFront = GetLightDepthBounds(
		context.m_ViewProjection,
		*pPointLight,
		minDepth,
		maxDepth);
	if (!isInFront)
	{
		return;
	}

	context.m_pRenderer->SetScissor(scissorRect);
	context.m_pRenderer->SetDepthBounds(true, minDepth, maxDepth);

	LightContext_t addLightContext;
	addLightContext.m_Count = 1;
	addLightContext.m_Lights[0] = *pPointLight;

	pManager->UpdateGlobalBuffers(
		context.m_LightSpaceMatrix,
		addLightContext,
		CF_NULL,
		true,
		0);

	TextureHandle hPointShadowMapTexture =
		context.m_pShadowResources->GetPointTexture();

	BlendState_t blendStateConfig;
	blendStateConfig.m_IsEnabled = true;
	blendStateConfig.m_IsAdditive = true;
	context.m_pRenderer->SetBlendState(blendStateConfig);

	for (int32 i = 0; i < context.m_pQueues->Count(); i++)
	{
		RenderQueueEntry_t& queueEntry = (*context.m_pQueues)[i];
		CUtlVector<RenderItem_t>& items = queueEntry.m_Items;
		if (items.IsEmpty())
		{
			continue;
		}

		if (queueEntry.m_QueueID == RenderQueue_t::Background)
		{
			continue;
		}

		::SetDepthStencilForQueue(
			*context.m_pRenderer,
			queueEntry.m_QueueID,
			true);

		for (int32 j = 0; j < items.Count(); j++)
		{
			const RenderItem_t& renderItem = items[j];
			CMaterial* pMaterial = ::GetMaterialForRenderItem(
				renderItem,
				*context.m_pRenderer);

			if (hPointShadowMapTexture != 0)
			{
				context.m_pRenderer->SetTexture(
					hPointShadowMapTexture,
					CF_BINDING_POINT_SHADOW_MAP,
					ShaderStageFragment,
					TextureWrapMode_t::ClampToEdge);
			}

			::DrawRenderItem(
				renderItem,
				*context.m_pRenderer,
				*context.m_pConstantBuffers,
				context.m_ViewProjection,
				pMaterial,
				true);
		}
	}

	context.m_pRenderer->SetScissor(Rect_t(0, 0, width, height));
	context.m_pRenderer->SetViewport(originalViewport);
	context.m_pRenderer->SetDepthBounds(false);

	BlendState_t restoreBlendState = BlendState_t();
	restoreBlendState.m_IsEnabled = false;
	context.m_pRenderer->SetBlendState(restoreBlendState);
}
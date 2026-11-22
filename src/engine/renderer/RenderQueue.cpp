//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "RenderQueue.h"
#include "ecs/components/Light.h"
#include "renderer/texture/Texture.h"
#include "dbg.h"
#include "utils/UtlMemory.h"
#include "renderer/passes/MainLightShadowPass.h"
#include "renderer/passes/PointLightShadowPass.h"
#include "renderer/passes/ForwardBasePass.h"
#include "renderer/passes/ForwardAddPass.h"

template<typename T>
static IRenderPass* CreatePass(const char* pName)
{
	void* pMemory = CUtlMemory::Alloc(sizeof(T));
	if (!pMemory)
	{
		Error("[CRenderQueueManager] Failed to allocate %s\n", pName);

		return CF_NULL;
	}

	return new(pMemory) T();
}

CRenderQueueManager::CRenderQueueManager() :
	m_pRenderer(CF_NULL),
	m_HasWarnedPointShadows(false)
{
}

CRenderQueueManager::~CRenderQueueManager()
{
	Shutdown();
}

void CRenderQueueManager::Init(IRenderer& renderer)
{
	m_pRenderer = &renderer;
	m_EnvironmentLighting.Init(renderer);
	m_ShadowResources.Init(renderer);
	m_ConstantBuffers.Init(renderer);

	m_Passes.SetCount(RenderPassType_t::Count);

	m_Passes[RenderPassType_t::MainLightShadow] =
		CreatePass<CMainLightShadowPass>("main light shadow pass");
	m_Passes[RenderPassType_t::PointLightShadow] =
		CreatePass<CPointLightShadowPass>("point light shadow pass");
	m_Passes[RenderPassType_t::ForwardBase] =
		CreatePass<CForwardBasePass>("forward base pass");
	m_Passes[RenderPassType_t::ForwardAdd] =
		CreatePass<CForwardAddPass>("forward add pass");
}

void CRenderQueueManager::Shutdown()
{
	for (int32 i = 0; i < m_Passes.Count(); i++)
	{
		if (m_Passes[i])
		{
			m_Passes[i]->~IRenderPass();
			CUtlMemory::Free(m_Passes[i]);
		}
	}
	m_Passes.RemoveAll();

	m_ShadowResources.Shutdown();
	m_pGlobalEnvironmentMap.Reset();
	m_EnvironmentLighting.Shutdown();
	m_pRenderer = CF_NULL;
	m_HasWarnedPointShadows = false;
}

void CRenderQueueManager::Clear()
{
	m_Queues.RemoveAll();
}

void CRenderQueueManager::AddItem(int32 queueID, const RenderItem_t& renderItem)
{
	int32 queueIndex = -1;
	for (int32 i = 0; i < m_Queues.Count(); i++)
	{
		if (m_Queues[i].m_QueueID == queueID)
		{
			queueIndex = i;

			break;
		}
	}

	if (queueIndex == -1)
	{
		queueIndex = m_Queues.AddToTail(RenderQueueEntry_t(queueID));
	}
	m_Queues[queueIndex].m_Items.AddToTail(renderItem);
}

void CRenderQueueManager::SortQueues()
{
	for (int32 i = 0; i < m_Queues.Count(); i++)
	{
		CUtlVector<RenderItem_t>& items = m_Queues[i].m_Items;
		int32 count = items.Count();
		if (count < 2)
		{
			continue;
		}

		bool isDescending = m_Queues[i].m_QueueID >= RenderQueue_t::Transparent;

		m_SortKeys.SetCount(count);
		for (int32 j = 0; j < count; j++)
		{
			m_SortKeys[j] = items[j].m_SortKey;
		}

		m_SortOrder.SetCount(count);
		m_RadixSort.SortIndices(
			m_SortKeys.Base(),
			m_SortOrder.Base(),
			count,
			isDescending);

		m_SortedItems.SetCount(count);
		for (int32 j = 0; j < count; j++)
		{
			m_SortedItems[j] = items[m_SortOrder[j]];
		}
		for (int32 j = 0; j < count; j++)
		{
			items[j] = m_SortedItems[j];
		}
	}
}

void CRenderQueueManager::SetLightContext(const LightContext_t& lights)
{
	m_LightContext = lights;
}

void CRenderQueueManager::SetGlobalEnvironmentMap(CTexture* pTexture)
{
	if (pTexture != m_pGlobalEnvironmentMap.Get())
	{
		if (pTexture && m_pRenderer)
		{
			m_EnvironmentLighting.Build(*pTexture);
		}
		else
		{
			m_EnvironmentLighting.Reset();
		}
	}

	m_pGlobalEnvironmentMap.Reset(pTexture);
}

void CRenderQueueManager::Render(
	IRenderer& renderer,
	const CMatrix4& viewProjection,
	const CCamera* pCamera)
{
	m_ConstantBuffers.UpdateCamera(renderer, pCamera);

	RenderPassContext_t context;
	context.m_pRenderer = &renderer;
	context.m_pConstantBuffers = &m_ConstantBuffers;
	context.m_pCamera = pCamera;
	context.m_ViewProjection = viewProjection;
	context.m_LightSpaceMatrix = CMatrix4(1.0f);
	context.m_pLightContext = &m_LightContext;
	context.m_pQueues = &m_Queues;
	context.m_pShadowResources = &m_ShadowResources;
	context.m_pEnvironmentLighting = &m_EnvironmentLighting;

	context.m_pGlobalEnvironmentMap = m_pGlobalEnvironmentMap.Get();
	context.m_CurrentLightIndex = -1;

	if (m_Passes.Count() >= RenderPassType_t::Count &&
		m_Passes[RenderPassType_t::MainLightShadow] &&
		m_Passes[RenderPassType_t::ForwardBase] &&
		m_Passes[RenderPassType_t::PointLightShadow] &&
		m_Passes[RenderPassType_t::ForwardAdd])
	{
		m_Passes[RenderPassType_t::MainLightShadow]->Render(context, this);

		m_Passes[RenderPassType_t::ForwardBase]->Render(context, this);

		int32 lightCount = m_LightContext.m_Count;
		int32 pointLightCount = 0;
		for (int32 i = 0; i < lightCount; i++)
		{
			if (m_LightContext.m_Lights[i].m_Type == LightType_t::Point)
			{
				pointLightCount++;
			}
		}

		if (pointLightCount > 1 && !m_HasWarnedPointShadows)
		{
			Warning(
				"[CRenderQueueManager] %d point lights redraw shadows "
				"every frame\n",
				pointLightCount);
			m_HasWarnedPointShadows = true;
		}

		for (int32 i = 0; i < lightCount; i++)
		{
			const Light_t& light = m_LightContext.m_Lights[i];
			if (light.m_Type == LightType_t::Point)
			{
				context.m_CurrentLightIndex = i;

				m_Passes[RenderPassType_t::PointLightShadow]->Render(
					context,
					this);

				m_Passes[RenderPassType_t::ForwardAdd]->Render(context, this);
			}
		}
	}
}

void CRenderQueueManager::UpdateGlobalBuffers(
	const CMatrix4& lightSpaceMatrix,
	const LightContext_t& lights,
	const Light_t* pDirLight,
	bool isAddPass,
	int32 currentLightIndex)
{
	float32 shadowMapResolution = float32(m_ShadowResources.GetMainSize());

	m_ConstantBuffers.UpdateScene(
		*m_pRenderer,
		lightSpaceMatrix,
		pDirLight,
		shadowMapResolution);
	m_ConstantBuffers.UpdateLighting(
		*m_pRenderer,
		lights,
		m_EnvironmentLighting,
		isAddPass,
		currentLightIndex);
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef RENDERQUEUE_H
#define RENDERQUEUE_H

#pragma once

#include "renderer/Renderer.h"
#include "renderer/RenderItem.h"
#include "WrenBind.h"
#include "utils/UtlVector.h"
#include "utils/RadixSort.h"
#include "utils/RefCounted.h"
#include "Matrix4.h"
#include "renderer/ibl/EnvironmentLighting.h"
#include "renderer/ConstantBuffers.h"
#include "renderer/ShadowResources.h"

class CMaterial;
class CTexture;
class CCamera;
class CRenderQueueManager;

struct RenderQueue_t
{
	enum Enum
	{
		Background = 4,
		Opaque = 8,
		Cutoff = 16,
		Transparent = 20,
		Overlay = 24
	};
};

struct Light_t
{
	CColor m_Color;
	CVector3 m_Position;
	CVector3 m_Direction;
	float32 m_Intensity;
	float32 m_Radius;
	float32 m_ShadowCenterOffset;
	float32 m_ShadowDistance;
	float32 m_ShadowOrthoSize;
	float32 m_ShadowNearPlane;
	float32 m_ShadowFarPlane;
	float32 m_ShadowResolution;
	float32 m_ShadowStrength;
	float32 m_ShadowBias;
	int32 m_Type;

	Light_t() :
		m_Color(1.0f, 1.0f, 1.0f),
		m_Position(0.0f, 0.0f, 0.0f),
		m_Direction(0.0f, -1.0f, 0.0f),
		m_Intensity(1.0f),
		m_Radius(10.0f),
		m_ShadowCenterOffset(30.0f),
		m_ShadowDistance(100.0f),
		m_ShadowOrthoSize(100.0f),
		m_ShadowNearPlane(-100.0f),
		m_ShadowFarPlane(300.0f),
		m_ShadowResolution(4096.0f),
		m_ShadowStrength(1.0f),
		m_ShadowBias(0.001f),
		m_Type(0)
	{
	}
};

struct LightContext_t
{
	Light_t m_Lights[CF_MAX_LIGHTS];
	int32 m_Count;

	LightContext_t() :
		m_Count(0)
	{
	}
};

struct RenderQueueEntry_t
{
	CUtlVector<RenderItem_t> m_Items;
	int32 m_QueueID;

	RenderQueueEntry_t() :
		m_QueueID(0)
	{
	}

	RenderQueueEntry_t(int32 queueID) :
		m_QueueID(queueID)
	{
	}
};

struct RenderPassType_t
{
	enum Enum
	{
		MainLightShadow = 0,
		PointLightShadow,
		ForwardBase,
		ForwardAdd,
		Count
	};
};

struct RenderPassContext_t
{
	CMatrix4 m_ViewProjection;
	CMatrix4 m_LightSpaceMatrix;
	IRenderer* m_pRenderer;
	CConstantBuffers* m_pConstantBuffers;
	const CCamera* m_pCamera;
	const LightContext_t* m_pLightContext;
	CUtlVector<RenderQueueEntry_t>* m_pQueues;
	const CShadowResources* m_pShadowResources;
	const CEnvironmentLighting* m_pEnvironmentLighting;
	CTexture* m_pGlobalEnvironmentMap;
	int32 m_CurrentLightIndex;
};

class IRenderPass
{
public:
	virtual ~IRenderPass()
	{
	}

	virtual void Render(
		RenderPassContext_t& context,
		CRenderQueueManager* pManager) = 0;
};

class CRenderQueueManager : public CWrenBindSingleton<CRenderQueueManager>
{
	WREN_SINGLETON_DECL()
public:
	CRenderQueueManager();
	~CRenderQueueManager();

	void Init(IRenderer& renderer);
	void Shutdown();
	void Clear() WREN_BIND_SINGLETON("clear()", s_pRenderQueue);
	void AddItem(int32 queueID, const RenderItem_t& renderItem);
	void SortQueues();
	void SetLightContext(const LightContext_t& lights);
	void SetGlobalEnvironmentMap(CTexture* pTexture) WREN_BIND_SINGLETON(
		"setGlobalEnvironmentMap(_)",
		s_pRenderQueue);
	void Render(
		IRenderer& renderer,
		const CMatrix4& viewProjection,
		const CCamera* pCamera = CF_NULL)
		WREN_BIND_FWD("render(_)", "WrenFwd_CRenderQueueManager_Render");

	void UpdateGlobalBuffers(
		const CMatrix4& lightSpaceMatrix,
		const LightContext_t& lights,
		const Light_t* pDirLight,
		bool isAddPass = false,
		int32 currentLightIndex = -1);

private:
	LightContext_t m_LightContext;
	CEnvironmentLighting m_EnvironmentLighting;
	CRadixSort m_RadixSort;
	CShadowResources m_ShadowResources;
	CConstantBuffers m_ConstantBuffers;

	CUtlVector<RenderQueueEntry_t> m_Queues;
	CUtlVector<IRenderPass*> m_Passes;
	CUtlVector<float32> m_SortKeys;
	CUtlVector<int32> m_SortOrder;
	CUtlVector<RenderItem_t> m_SortedItems;

	CRef<CTexture> m_pGlobalEnvironmentMap;
	IRenderer* m_pRenderer;
	bool m_HasWarnedPointShadows;
};

#endif // RENDERQUEUE_H
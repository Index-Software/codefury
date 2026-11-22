//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SCENE_H
#define SCENE_H

#pragma once

#include "renderer/Renderer.h"
#include "WrenBind.h"
#include "renderer/Skybox.h"
#include "utils/UtlVector.h"

class CRenderQueueManager;
class CEntityRegistry;
class CCamera;

class CScene : public CWrenBindSingleton<CScene>
{
	WREN_SINGLETON_DECL()
public:
	CScene() :
		m_pRegistry(CF_NULL),
		m_hMainCamera(0)
	{
	}

	explicit CScene(CEntityRegistry* pRegistry) :
		m_pRegistry(pRegistry),
		m_hMainCamera(0)
	{
	}

	~CScene()
	{
		Clear();
	}

	void AddEntity(uint32 hID) WREN_BIND_SINGLETON("addEntity(_)", s_pScene);
	void RemoveEntity(uint32 hID)
		WREN_BIND_SINGLETON("removeEntity(_)", s_pScene);
	void Clear() WREN_BIND_SINGLETON("clear()", s_pScene);
	bool Contains(uint32 hID) const
		WREN_BIND_SINGLETON("contains(_)", s_pScene);

	void SetSkybox(CSkybox* pSkybox)
		WREN_BIND_SINGLETON("setSkybox(_)", s_pScene);
	CRef<CSkybox> GetSkybox() const WREN_BIND_SINGLETON("getSkybox", s_pScene)
	{
		return m_pSkybox;
	}

	void SetMainCamera(uint32 hEntity) WREN_BIND_SINGLETON(
		"setMainCamera(_)",
		s_pScene)
	{
		m_hMainCamera = hEntity;
	}

	uint32 GetMainCameraHandle() const WREN_BIND_SINGLETON(
		"getMainCamera", s_pScene)
	{
		return m_hMainCamera;
	}

	void Update(
		CRenderQueueManager& renderQueueManager,
		IRenderer& renderer) WREN_BIND_SINGLETON("update()", s_pScene);

	void Render(
		CRenderQueueManager& renderQueueManager,
		IRenderer& renderer) WREN_BIND_SINGLETON("render()", s_pScene);

private:
	CCamera* GetMainCamera() const;

	CUtlVector<uint32> m_Entities;
	CEntityRegistry* m_pRegistry;
	CRef<CSkybox> m_pSkybox;
	uint32 m_hMainCamera;
};

#endif // SCENE_H
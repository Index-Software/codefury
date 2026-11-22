//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Scene.h"
#include "ecs/EntityRegistry.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/Transform.h"
#include "ecs/components/Camera.h"
#include "ecs/components/Light.h"
#include "mathsfury/Maths.h"
#include "renderer/material/Material.h"
#include "renderer/RenderQueue.h"

void CScene::AddEntity(uint32 hID)
{
	if (hID == 0)
	{
		return;
	}

	if (m_Entities.Find(hID) == -1)
	{
		m_Entities.AddToTail(hID);
	}
}

void CScene::RemoveEntity(uint32 hID)
{
	int32 index = m_Entities.Find(hID);
	if (index != -1)
	{
		m_Entities.RemoveAt(index);
	}
}

void CScene::Clear()
{
	if (m_pRegistry)
	{
		for (int32 i = 0; i < m_Entities.Count(); i++)
		{
			m_pRegistry->DestroyEntity(m_Entities[i]);
		}
	}
	m_Entities.RemoveAll();
}

bool CScene::Contains(uint32 hID) const
{
	return m_Entities.Find(hID) != -1;
}

void CScene::SetSkybox(CSkybox* pSkybox)
{
	m_pSkybox.Reset(pSkybox);
}

void CScene::Update(
	CRenderQueueManager& renderQueueManager,
	IRenderer& renderer)
{
	CCamera* pCameraComponent = GetMainCamera();
	if (!pCameraComponent)
	{
		return;
	}

	CVector3 cameraPos = pCameraComponent->GetPosition();

	if (m_pSkybox && m_pSkybox->GetMesh())
	{
		m_pSkybox->SetPosition(cameraPos);

		CMaterial* pSkyMaterial = m_pSkybox->GetMaterial();
		if (pSkyMaterial)
		{
			CTexture* pSkyTexture = pSkyMaterial->GetTexture().Get();
			if (!pSkyTexture && pSkyMaterial->GetShader())
			{
				pSkyTexture = pSkyMaterial->GetShader()->GetTexture(0).Get();
			}

			if (pSkyTexture)
			{
				renderQueueManager.SetGlobalEnvironmentMap(pSkyTexture);
			}
		}

		RenderItem_t skybox(
			0,
			m_pSkybox->GetMesh(),
			m_pSkybox->GetTransform(),
			0.0f,
			-1);
		renderQueueManager.AddItem(RenderQueue_t::Background, skybox);
	}

	LightContext_t lightContext;
	for (int32 entityIndex = 0; entityIndex < m_Entities.Count(); entityIndex++)
	{
		uint32 hID = m_Entities[entityIndex];
		CLight* pLight = m_pRegistry->GetComponent<CLight>(hID);
		if (!pLight || lightContext.m_Count >= CF_MAX_LIGHTS)
		{
			continue;
		}

		CTransform* pTransform = m_pRegistry->GetComponent<CTransform>(hID);
		Light_t& light = lightContext.m_Lights[lightContext.m_Count++];
		light.m_Position = pTransform
			? pTransform->m_Position
			: pLight->m_Position;
		light.m_Direction = pLight->m_Direction;
		light.m_Color = pLight->m_Color;
		light.m_Intensity = pLight->m_Intensity;
		light.m_Radius = pLight->m_Radius;
		light.m_ShadowCenterOffset = pLight->m_ShadowCenterOffset;
		light.m_ShadowDistance = pLight->m_ShadowDistance;
		light.m_ShadowOrthoSize = pLight->m_ShadowOrthoSize;
		light.m_ShadowNearPlane = pLight->m_ShadowNearPlane;
		light.m_ShadowFarPlane = pLight->m_ShadowFarPlane;
		light.m_ShadowResolution = pLight->m_ShadowResolution;
		light.m_ShadowStrength = pLight->m_ShadowStrength;
		light.m_ShadowBias = pLight->m_ShadowBias;
		light.m_Type = pLight->m_Type;
	}

	renderQueueManager.SetLightContext(lightContext);

	for (int32 entityIndex = 0; entityIndex < m_Entities.Count(); entityIndex++)
	{
		uint32 hID = m_Entities[entityIndex];
		CMesh* pMesh = m_pRegistry->GetComponent<CMesh>(hID);
		CTransform* pTransform = m_pRegistry->GetComponent<CTransform>(hID);

		if (!pMesh)
		{
			continue;
		}

		CVector3 entityPosition = pTransform
			? pTransform->m_Position
			: CVector3(0.0f, 0.0f, 0.0f);
		CVector3 cameraOffset = entityPosition - cameraPos;
		float32 sortKey = CMaths::Dot(cameraOffset, cameraOffset);

		if (pMesh->HasSubMeshes())
		{
			const CUtlVector<SubMesh_t>& subMeshes = pMesh->GetSubMeshes();
			for (int32 i = 0; i < subMeshes.Count(); i++)
			{
				const SubMesh_t& subMesh = subMeshes[i];

				int32 queue = subMesh.m_pMaterial
					? subMesh.m_pMaterial->GetRenderQueue()
					: 0;
				RenderItem_t item(
					hID,
					pMesh,
					pTransform,
					sortKey,
					static_cast<int32>(i));
				renderQueueManager.AddItem(queue, item);
			}
		}
		else
		{
			int32 queue = 0;
			const CUtlVector<CMaterial*>& materials = pMesh->GetMaterials();
			CMaterial* pMaterial = materials.IsEmpty() ? CF_NULL : materials[0];
			if (!pMaterial)
			{
				pMesh->SetMaterial(GetDefaultMaterial(&renderer));

				const CUtlVector<CMaterial*>& updatedMaterials =
					pMesh->GetMaterials();
				pMaterial = updatedMaterials.IsEmpty()
					? CF_NULL
					: updatedMaterials[0];
			}

			if (pMaterial)
			{
				queue = pMaterial->GetRenderQueue();
			}

			RenderItem_t item(hID, pMesh, pTransform, sortKey, -1);
			renderQueueManager.AddItem(queue, item);
		}
	}
}

void CScene::Render(
	CRenderQueueManager& renderQueueManager,
	IRenderer& renderer)
{
	renderQueueManager.Clear();

	Update(renderQueueManager, renderer);

	renderQueueManager.SortQueues();

	CCamera* pCamera = GetMainCamera();
	if (pCamera)
	{
		renderQueueManager.Render(
			renderer,
			pCamera->GetViewProjectionMatrix(),
			pCamera);
	}
}

CCamera* CScene::GetMainCamera() const
{
	if (m_hMainCamera != 0)
	{
		return m_pRegistry
			? m_pRegistry->GetCameraComponent(m_hMainCamera)
			: CF_NULL;
	}

	for (int32 i = 0; i < m_Entities.Count(); i++)
	{
		CCamera* pCamera = m_pRegistry
			? m_pRegistry->GetCameraComponent(m_Entities[i])
			: CF_NULL;
		if (pCamera)
		{
			return pCamera;
		}
	}

	if (m_pRegistry)
	{
		CEntity cameraEntity = m_pRegistry->FindEntityByName("Camera");
		if (cameraEntity.IsValid())
		{
			return m_pRegistry->GetCameraComponent(cameraEntity.GetID());
		}
	}

	return CF_NULL;
}
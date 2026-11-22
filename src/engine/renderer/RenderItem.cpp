//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/RenderItem.h"
#include "RenderQueue.h"
#include "renderer/ConstantBuffers.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/Transform.h"
#include "renderer/material/Material.h"
#include "renderer/shader/Shader.h"
#include "renderer/texture/Texture.h"
#include "mathsfury/Maths.h"
#include "config/ConfigVar.h"

static CConfigVar s_rFrustumCulling(
	"r_frustumculling",
	true,
	ConfigVarNone);

void SetDepthStencilForQueue(
	IRenderer& renderer,
	int32 queueID,
	bool overrideDepthWrite)
{
	DepthStencilState_t depthStencilState;
	if (queueID >= RenderQueue_t::Transparent)
	{
		depthStencilState.m_IsDepthTest = false;
		depthStencilState.m_IsDepthWrite = false;
	}
	else if (queueID >= RenderQueue_t::Cutoff)
	{
		depthStencilState.m_IsDepthTest = true;
		depthStencilState.m_IsDepthWrite = false;
	}
	else if (queueID == RenderQueue_t::Background)
	{
		depthStencilState.m_IsDepthTest = true;
		depthStencilState.m_IsDepthWrite = false;
	}
	else
	{
		depthStencilState.m_IsDepthTest = true;
		depthStencilState.m_IsDepthWrite = overrideDepthWrite ? false : true;
	}

	renderer.SetDepthStencilState(depthStencilState);
}

CMaterial* GetMaterialForRenderItem(
	const RenderItem_t& renderItem,
	IRenderer& renderer)
{
	CMaterial* pMaterial = CF_NULL;

	if (renderItem.m_SubMeshIndex >= 0 && renderItem.m_pMesh->HasSubMeshes())
	{
		const CUtlVector<SubMesh_t>& subMeshes =
			renderItem.m_pMesh->GetSubMeshes();
		if (renderItem.m_SubMeshIndex < subMeshes.Count())
		{
			pMaterial = subMeshes[renderItem.m_SubMeshIndex].m_pMaterial;
			if (!pMaterial)
			{
				const CUtlVector<CMaterial*>& materials =
					renderItem.m_pMesh->GetMaterials();
				pMaterial = materials.IsEmpty() ? CF_NULL : materials[0];
			}
		}
	}
	else
	{
		const CUtlVector<CMaterial*>& materials =
			renderItem.m_pMesh->GetMaterials();
		pMaterial = materials.IsEmpty() ? CF_NULL : materials[0];
	}

	return pMaterial ? pMaterial : GetDefaultMaterial(&renderer);
}

void DrawRenderItem(
	const RenderItem_t& renderItem,
	IRenderer& renderer,
	CConstantBuffers& constantBuffers,
	const CMatrix4& viewProjection,
	CMaterial* pMaterial,
	bool isAddPass)
{
	CMatrix4 model = CMatrix4(1.0f);
	if (renderItem.m_pTransform)
	{
		model = renderItem.m_pTransform->GetMatrix();
	}

	CMaterial* pShaderMaterial = pMaterial;
	if (!pShaderMaterial || !pShaderMaterial->GetShader())
	{
		pShaderMaterial = GetDefaultMaterial(&renderer);
	}

	CMatrix4 mvp = viewProjection * model;

	CShader* pShader = pShaderMaterial->GetShader();
	const bool isSkybox = pShader &&
		pShader->GetType() == ShaderType_t::Skybox;
	if (!isSkybox && s_rFrustumCulling.GetBool())
	{
		Plane_t frustumPlanes[6];
		renderer.ExtractFrustumPlanes(mvp, frustumPlanes);
		if (!renderer.TestAABBFrustum(
			renderItem.m_pMesh->GetAABBCenter(),
			renderItem.m_pMesh->GetAABBExtent(),
			frustumPlanes))
		{
			return;
		}
	}

	if (pShader)
	{
		renderer.SetShaderProgram(isAddPass
			? pShader->GetAddPassHandle()
			: pShader->GetHandle());

		constantBuffers.BindShared(renderer);

		const CUtlVector<CTexture*>& shaderTextures = pShader->GetTextures();
		for (int32 textureIndex = 0;
			textureIndex < shaderTextures.Count();
			textureIndex++)
		{
			CTexture* pShaderTexture = shaderTextures[textureIndex];
			if (pShaderTexture)
			{
				pShaderTexture->Bind(uint32(textureIndex));
			}
		}

		CTexture* pMaterialTexture = pShaderMaterial->GetTexture().Get();
		if (pMaterialTexture)
		{
			pMaterialTexture->Bind(0);
		}

		pShaderMaterial->FlushProperties();
	}

	if (!isSkybox)
	{
		constantBuffers.SetModel(renderer, mvp, model);
	}

	if (renderItem.m_SubMeshIndex >= 0 && renderItem.m_pMesh->HasSubMeshes())
	{
		const CUtlVector<SubMesh_t>& subMeshes =
			renderItem.m_pMesh->GetSubMeshes();
		if (renderItem.m_SubMeshIndex < subMeshes.Count())
		{
			const SubMesh_t& subMesh = subMeshes[renderItem.m_SubMeshIndex];

			uint32 stride = renderItem.m_pMesh->GetLayout().GetStride();
			if (stride == 0)
			{
				stride = 3 * sizeof(float32);
			}

			renderer.SetVertexBuffer(
				renderItem.m_pMesh->m_hVertexBuffer,
				0,
				stride,
				0,
				&renderItem.m_pMesh->GetLayout());

			if (subMesh.m_IndexCount > 0 && renderItem.m_pMesh->m_hIndexBuffer)
			{
				renderer.SetIndexBuffer(renderItem.m_pMesh->m_hIndexBuffer, 0);

				renderer.DrawIndexed(
					subMesh.m_IndexCount,
					subMesh.m_IndexOffset,
					subMesh.m_VertexOffset);
			}
			else
			{
				renderer.Draw(subMesh.m_VertexCount, subMesh.m_VertexOffset);
			}
		}
	}
	else
	{
		renderItem.m_pMesh->Draw(renderer);
	}
}
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Skybox.h"
#include "MeshBuilder.h"
#include "RenderQueue.h"
#include "mathsfury/Maths.h"

CSkybox::CSkybox(IRenderer* pRenderer) :
	m_pRenderer(pRenderer),
	m_pTransform(CRef<CTransform>::Create())
{
}

bool CSkybox::Init()
{
	m_pTransform->m_Scale = CVector3(1.0f, 1.0f, 1.0f);
	m_pTransform->m_Position = CVector3(0.0f, 0.0f, 0.0f);

	return true;
}

bool CSkybox::SetMaterial(CMaterial* pMaterial)
{
	if (!pMaterial)
	{
		return false;
	}

	CShader* pShader = pMaterial->GetShader();
	if (!pShader)
	{
		Warning("[CSkybox] Material has no shader\n");

		return false;
	}

	if (pShader->GetType() != ShaderType_t::Skybox)
	{
		Warning("[CSkybox] Material shader type is not Skybox\n");
	}

	m_pMaterial.Reset(pMaterial);
	m_pMaterial->SetRenderQueue(RenderQueue_t::Background);

	if (pShader->GetType() == ShaderType_t::Skybox)
	{
		CVertexLayout quadLayout;
		quadLayout.AddAttribute(
			"position",
			uint32(VertexFormat_t::Float2),
			0,
			VertexSemantic_t::Position);
		quadLayout.SetStride(sizeof(float32) * 2);

		m_pMesh = CRef<CMesh>::Create(quadLayout);
		if (!m_pMesh.IsValid())
		{
			return false;
		}

		CUtlVector<float32> verts;
		verts.AddToTail(-1.0f);
		verts.AddToTail(-1.0f);

		verts.AddToTail(1.0f);
		verts.AddToTail(-1.0f);

		verts.AddToTail(1.0f);
		verts.AddToTail(1.0f);

		verts.AddToTail(-1.0f);
		verts.AddToTail(1.0f);

		CUtlVector<uint32> indices;
		indices.AddToTail(0);
		indices.AddToTail(1);
		indices.AddToTail(2);

		indices.AddToTail(0);
		indices.AddToTail(2);
		indices.AddToTail(3);

		m_pMesh->SetVertices(verts);
		m_pMesh->SetIndices(indices);
		m_pMesh->SetMaterial(m_pMaterial.Get());
		if (!m_pMesh->Upload(*m_pRenderer))
		{
			AssertMsg(false, "[CSkybox] Mesh upload failed");

			return false;
		}

		m_pTransform->m_Scale = CVector3(1.0f, 1.0f, 1.0f);
	}
	else
	{
		m_pMesh = CMeshBuilder::CreateCube(
			m_pRenderer,
			m_pMaterial.Get(),
			0.5f,
			CColor(1.0f, 1.0f, 1.0f, 1.0f),
			FaceOrientation_t::Inwards);
		if (!m_pMesh.IsValid())
		{
			return false;
		}

		m_pTransform->m_Scale = CVector3(1.0f, 1.0f, 1.0f);
	}

	return true;
}

void CSkybox::SetPosition(const CVector3& position)
{
	m_pTransform->m_Position = position;
}
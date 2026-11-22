//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef MESH_H
#define MESH_H

#pragma once

#include "../EntityComponent.h"
#include "WrenBind.h"
#include "renderer/Renderer.h"
#include "renderer/material/Material.h"
#include "utils/UtlVector.h"
#include "mathsfury/Vector3.h"

struct SubMesh_t
{
	uint32 m_VertexOffset;
	uint32 m_VertexCount;
	uint32 m_IndexOffset;
	uint32 m_IndexCount;
	CMaterial* m_pMaterial;

	SubMesh_t() :
		m_VertexOffset(0),
		m_VertexCount(0),
		m_IndexOffset(0),
		m_IndexCount(0),
		m_pMaterial(CF_NULL)
	{
	}

	SubMesh_t(
		uint32 vOffset,
		uint32 vCount,
		uint32 iOffset,
		uint32 iCount,
		CMaterial* pMat = CF_NULL) :
		m_VertexOffset(vOffset),
		m_VertexCount(vCount),
		m_IndexOffset(iOffset),
		m_IndexCount(iCount),
		m_pMaterial(pMat)
	{
	}
};

class CMesh : public CEntityComponent
{
public:
	CMesh() : m_hVertexBuffer(0), m_hIndexBuffer(0)
	{
	}

	CMesh(const CVertexLayout& layout) :
		m_hVertexBuffer(0),
		m_hIndexBuffer(0),
		m_Layout(layout)
	{
	}

	CMesh(const CMesh&);
	CMesh& operator=(const CMesh&);

	virtual const char* TypeName() const CF_OVERRIDE
	{
		return "CMesh";
	}

	void SetVertices(const CUtlVector<float32>& verts)
	{
		m_Vertices = verts;
	}

	void SetIndices(const CUtlVector<uint32>& indices)
	{
		m_Indices = indices;
	}

	const CVertexLayout& GetLayout() const
	{
		return m_Layout;
	}

	void AddSubMesh(
		uint32 vertexOffset,
		uint32 vertexCount,
		uint32 indexOffset,
		uint32 indexCount,
		CMaterial* pMaterial = CF_NULL)
	{
		m_SubMeshes.AddToTail(
			SubMesh_t(
				vertexOffset,
				vertexCount,
				indexOffset,
				indexCount,
				pMaterial));
		if (pMaterial)
		{
			m_Materials.Add(pMaterial);
		}
	}

	const CUtlVector<SubMesh_t>& GetSubMeshes() const
	{
		return m_SubMeshes;
	}

	bool HasSubMeshes() const
	{
		return m_SubMeshes.Count() > 0;
	}

	void SetMaterial(CMaterial* pMaterial)
	{
		m_Materials.Assign(pMaterial);
	}

	const CUtlVector<CMaterial*>& GetMaterials() const
	{
		return m_Materials.Items();
	}

	bool Upload(IRenderer& renderer);
	void Draw(IRenderer& renderer) const;

	const CVector3& GetAABBCenter() const WREN_BIND("getAABBCenter")
	{
		return m_AABBCenter;
	}
	const CVector3& GetAABBExtent() const WREN_BIND("getAABBExtent")
	{
		return m_AABBExtent;
	}

	BufferHandle m_hVertexBuffer;
	BufferHandle m_hIndexBuffer;
private:
	CVertexLayout m_Layout;
	CUtlVector<float32> m_Vertices;
	CUtlVector<uint32> m_Indices;
	CUtlVector<SubMesh_t> m_SubMeshes;
	CRefList<CMaterial> m_Materials;

	CVector3 m_AABBCenter;
	CVector3 m_AABBExtent;
};

#endif // MESH_H
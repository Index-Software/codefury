//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Mesh.h"
#include "mathsfury/Vector3.h"
#include <stdio.h>
#include <string.h>

bool CMesh::Upload(IRenderer& renderer)
{
	if (m_hVertexBuffer)
	{
		renderer.DestroyBuffer(m_hVertexBuffer);
		m_hVertexBuffer = 0;
	}

	if (m_hIndexBuffer)
	{
		renderer.DestroyBuffer(m_hIndexBuffer);
		m_hIndexBuffer = 0;
	}

	if (m_Vertices.Count() > 0)
	{
		uint32 bytes = m_Vertices.Count() * sizeof(float32);
		m_hVertexBuffer = renderer.CreateVertexBuffer(
			m_Vertices.Base(),
			bytes,
			BufferUsage_t::Static);
		if (!m_hVertexBuffer)
		{
			return false;
		}
	}

	if (m_Indices.Count() > 0)
	{
		uint32 bytes = m_Indices.Count() * sizeof(uint32);
		m_hIndexBuffer = renderer.CreateIndexBuffer(
			m_Indices.Base(),
			bytes,
			IndexFormat_t::UInt32,
			BufferUsage_t::Static);
		if (!m_hIndexBuffer)
		{
			return false;
		}
	}

	uint32 stride = m_Layout.GetStride();
	uint32 floatsPerVertex = (stride > 0) ? (stride / sizeof(float32)) : 3;
	int32 vertexCount = (floatsPerVertex > 0)
		? (m_Vertices.Count() / static_cast<int32>(floatsPerVertex))
		: 0;

	uint32 positionOffset = 0;
	uint32 positionComponents = 3;
	const CUtlVector<VertexAttribute_t>& attributes = m_Layout.GetAttributes();
	for (int32 i = 0; i < attributes.Count(); i++)
	{
		const VertexAttribute_t& attribute = attributes[i];
		if (attribute.m_VertexSemantic != VertexSemantic_t::Position)
		{
			continue;
		}

		positionOffset = attribute.m_Offset / sizeof(float32);
		switch (attribute.m_Format)
		{
			case VertexFormat_t::Float:
				positionComponents = 1;

				break;
			case VertexFormat_t::Float2:
				positionComponents = 2;

				break;
			case VertexFormat_t::Float3:
				positionComponents = 3;

				break;
			case VertexFormat_t::Float4:
				positionComponents = 4;

				break;
			default:
				positionComponents = 0;

				break;
		}

		break;
	}

	if (vertexCount > 0 && positionComponents > 0 &&
		positionOffset + positionComponents <= floatsPerVertex)
	{
		CVector3 minPos(
			m_Vertices[positionOffset],
			positionComponents > 1 ? m_Vertices[positionOffset + 1] : 0.0f,
			positionComponents > 2 ? m_Vertices[positionOffset + 2] : 0.0f);
		CVector3 maxPos = minPos;

		for (int32 i = 1; i < vertexCount; i++)
		{
			int32 base = i * static_cast<int32>(floatsPerVertex)
				+ static_cast<int32>(positionOffset);
			float32 x = m_Vertices[base];
			float32 y = positionComponents > 1 ? m_Vertices[base + 1] : 0.0f;
			float32 z = positionComponents > 2 ? m_Vertices[base + 2] : 0.0f;

			if (x < minPos.m_X)
			{
				minPos.m_X = x;
			}
			if (y < minPos.m_Y)
			{
				minPos.m_Y = y;
			}
			if (z < minPos.m_Z)
			{
				minPos.m_Z = z;
			}
			if (x > maxPos.m_X)
			{
				maxPos.m_X = x;
			}
			if (y > maxPos.m_Y)
			{
				maxPos.m_Y = y;
			}
			if (z > maxPos.m_Z)
			{
				maxPos.m_Z = z;
			}
		}

		m_AABBCenter = CVector3(
			(minPos.m_X + maxPos.m_X) * 0.5f,
			(minPos.m_Y + maxPos.m_Y) * 0.5f,
			(minPos.m_Z + maxPos.m_Z) * 0.5f);
		m_AABBExtent = CVector3(
			(maxPos.m_X - minPos.m_X) * 0.5f,
			(maxPos.m_Y - minPos.m_Y) * 0.5f,
			(maxPos.m_Z - minPos.m_Z) * 0.5f);
	}

	return true;
}

void CMesh::Draw(IRenderer& renderer) const
{
	if (!m_hVertexBuffer)
	{
		return;
	}

	uint32 stride = m_Layout.GetStride();
	if (stride == 0)
	{
		stride = 3 * sizeof(float32);
	}

	renderer.SetVertexBuffer(m_hVertexBuffer, 0, stride, 0, &m_Layout);

	if (HasSubMeshes())
	{
		if (m_hIndexBuffer)
		{
			renderer.SetIndexBuffer(m_hIndexBuffer, 0);
			for (int32 i = 0; i < m_SubMeshes.Count(); i++)
			{
				const SubMesh_t& subMesh = m_SubMeshes[i];
				renderer.DrawIndexed(
					subMesh.m_IndexCount,
					subMesh.m_IndexOffset,
					subMesh.m_VertexOffset);
			}
		}
		else
		{
			for (int32 i = 0; i < m_SubMeshes.Count(); i++)
			{
				const SubMesh_t& subMesh = m_SubMeshes[i];
				renderer.Draw(subMesh.m_VertexCount, subMesh.m_VertexOffset);
			}
		}
	}
	else
	{
		if (m_hIndexBuffer)
		{
			renderer.SetIndexBuffer(m_hIndexBuffer, 0);
			renderer.DrawIndexed(m_Indices.Count(), 0, 0);
		}
		else
		{
			uint32 vertexCount = 0;
			if (stride > 0)
			{
				vertexCount = (m_Vertices.Count() * sizeof(float32)) / stride;
			}

			if (vertexCount == 0)
			{
				return;
			}

			renderer.Draw(vertexCount, 0);
		}
	}
}
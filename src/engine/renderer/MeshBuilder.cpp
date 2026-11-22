//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "MeshBuilder.h"
#include "ecs/components/Mesh.h"
#include "renderer/material/Material.h"
#include "utils/UtlVector.h"
#include "dbg.h"
#include "mathsfury/Maths.h"
#include "mathsfury/Vector2.h"
#include "mathsfury/Vector3.h"

#include <math.h>
#include <string.h>

struct MeshBuilderVertex_t
{
	CVector3 m_Position;
	CVector3 m_Normal;
	CVector2 m_TexCoord;
	uint32 m_Color;

	MeshBuilderVertex_t()
	{
	}

	MeshBuilderVertex_t(
		const CVector3& pos,
		const CVector3& normal,
		const CVector2& uv,
		uint32 color) :
		m_Position(pos),
		m_Normal(normal),
		m_TexCoord(uv),
		m_Color(color)
	{
	}
};

static CVertexLayout BuildLitLayout()
{
	CVertexLayout layout;
	layout.AddAttribute(
		"position",
		uint32(VertexFormat_t::Float3),
		0,
		VertexSemantic_t::Position);
	layout.AddAttribute(
		"texCoord",
		uint32(VertexFormat_t::Float2),
		sizeof(CVector3) + sizeof(CVector3),
		VertexSemantic_t::TexCoord0);
	layout.AddAttribute(
		"normal",
		uint32(VertexFormat_t::Float3),
		sizeof(CVector3),
		VertexSemantic_t::Normal);
	layout.AddAttribute(
		"color",
		uint32(VertexFormat_t::UByte4_Norm),
		sizeof(CVector3) + sizeof(CVector3) + sizeof(CVector2),
		VertexSemantic_t::Color0);
	layout.SetStride(sizeof(MeshBuilderVertex_t));

	return layout;
}

static CRef<CMesh> CreateMesh(
	IRenderer* pRenderer,
	CMaterial* pMaterial,
	const CUtlVector<MeshBuilderVertex_t>& verts,
	const CUtlVector<uint32>& indices)
{
	CUtlVector<float32> vertexData;
	vertexData.SetCount(
		int32(uint64(verts.Count()) * sizeof(MeshBuilderVertex_t) /
			sizeof(float32)));
	memcpy(
		vertexData.Base(),
		verts.Base(),
		uint64(verts.Count()) * sizeof(MeshBuilderVertex_t));

	CRef<CMesh> pMesh = CRef<CMesh>::Create(BuildLitLayout());
	if (!pMesh.IsValid())
	{
		return pMesh;
	}

	pMesh->SetVertices(vertexData);
	pMesh->SetIndices(indices);
	pMesh->SetMaterial(pMaterial);
	if (!pMesh->Upload(*pRenderer))
	{
		Error("[CMeshBuilder] Mesh upload failed\n");

		return CRef<CMesh>();
	}

	return pMesh;
}

static void CreateFace(
	CUtlVector<MeshBuilderVertex_t>& verts,
	CUtlVector<uint32>& indices,
	const CVector3& bottomLeft,
	const CVector3& bottomRight,
	const CVector3& topLeft,
	const CVector3& topRight,
	const CVector3& normal,
	uint32 packedColor,
	int32 subdivisions,
	FaceOrientation_t::Enum faceOrientation)
{
	if (subdivisions < 1)
	{
		Error("[CMeshBuilder] Invalid subdivisions %d\n", subdivisions);

		subdivisions = 1;
	}

	int32 startVert = verts.Count();

	for (int32 y = 0; y <= subdivisions; y++)
	{
		float32 v = float32(y) / float32(subdivisions);
		for (int32 x = 0; x <= subdivisions; x++)
		{
			float32 u = float32(x) / float32(subdivisions);

			CVector3 pos =
				bottomLeft * (1.0f - u) * (1.0f - v) +
				bottomRight * u * (1.0f - v) +
				topLeft * (1.0f - u) * v +
				topRight * u * v;

			CVector3 finalNormal = normal;
			if (faceOrientation == FaceOrientation_t::Inwards)
			{
				finalNormal = CVector3(-normal.m_X, -normal.m_Y, -normal.m_Z);
			}

			verts.AddToTail(MeshBuilderVertex_t(
				pos,
				finalNormal,
				CVector2(u, v),
				packedColor));
		}
	}

	for (int32 y = 0; y < subdivisions; y++)
	{
		for (int32 x = 0; x < subdivisions; x++)
		{
			int32 i0 = startVert + y * (subdivisions + 1) + x;
			int32 i1 = startVert + y * (subdivisions + 1) + (x + 1);
			int32 i2 = startVert + (y + 1) * (subdivisions + 1) + (x + 1);
			int32 i3 = startVert + (y + 1) * (subdivisions + 1) + x;

			if (faceOrientation == FaceOrientation_t::Outwards)
			{
				indices.AddToTail(uint32(i0));
				indices.AddToTail(uint32(i1));
				indices.AddToTail(uint32(i2));

				indices.AddToTail(uint32(i0));
				indices.AddToTail(uint32(i2));
				indices.AddToTail(uint32(i3));
			}
			else
			{
				indices.AddToTail(uint32(i0));
				indices.AddToTail(uint32(i2));
				indices.AddToTail(uint32(i1));

				indices.AddToTail(uint32(i0));
				indices.AddToTail(uint32(i3));
				indices.AddToTail(uint32(i2));
			}
		}
	}
}

CRef<CMesh> CMeshBuilder::CreateCube(
	IRenderer* pRenderer,
	CMaterial* pMaterial,
	float32 halfSize,
	const CColor& color,
	FaceOrientation_t::Enum faceOrientation,
	int32 subdivisions)
{
	const uint32 packedColor = CColor::PackColor(color);

	CUtlVector<MeshBuilderVertex_t> verts;
	CUtlVector<uint32> indices;

	// Front face
	CreateFace(
		verts,
		indices,
		CVector3(-halfSize, -halfSize, halfSize),
		CVector3(halfSize, -halfSize, halfSize),
		CVector3(-halfSize, halfSize, halfSize),
		CVector3(halfSize, halfSize, halfSize),
		CVector3(0.0f, 0.0f, 1.0f),
		packedColor,
		subdivisions,
		faceOrientation);

	// Back face
	CreateFace(
		verts,
		indices,
		CVector3(halfSize, -halfSize, -halfSize),
		CVector3(-halfSize, -halfSize, -halfSize),
		CVector3(halfSize, halfSize, -halfSize),
		CVector3(-halfSize, halfSize, -halfSize),
		CVector3(0.0f, 0.0f, -1.0f),
		packedColor,
		subdivisions,
		faceOrientation);

	// Top face
	CreateFace(
		verts,
		indices,
		CVector3(-halfSize, halfSize, halfSize),
		CVector3(halfSize, halfSize, halfSize),
		CVector3(-halfSize, halfSize, -halfSize),
		CVector3(halfSize, halfSize, -halfSize),
		CVector3(0.0f, 1.0f, 0.0f),
		packedColor,
		subdivisions,
		faceOrientation);

	// Bottom face
	CreateFace(
		verts,
		indices,
		CVector3(-halfSize, -halfSize, -halfSize),
		CVector3(halfSize, -halfSize, -halfSize),
		CVector3(-halfSize, -halfSize, halfSize),
		CVector3(halfSize, -halfSize, halfSize),
		CVector3(0.0f, -1.0f, 0.0f),
		packedColor,
		subdivisions,
		faceOrientation);

	// Right face
	CreateFace(
		verts,
		indices,
		CVector3(halfSize, -halfSize, halfSize),
		CVector3(halfSize, -halfSize, -halfSize),
		CVector3(halfSize, halfSize, halfSize),
		CVector3(halfSize, halfSize, -halfSize),
		CVector3(1.0f, 0.0f, 0.0f),
		packedColor,
		subdivisions,
		faceOrientation);

	// Left face
	CreateFace(
		verts,
		indices,
		CVector3(-halfSize, -halfSize, -halfSize),
		CVector3(-halfSize, -halfSize, halfSize),
		CVector3(-halfSize, halfSize, -halfSize),
		CVector3(-halfSize, halfSize, halfSize),
		CVector3(-1.0f, 0.0f, 0.0f),
		packedColor,
		subdivisions,
		faceOrientation);

	return CreateMesh(pRenderer, pMaterial, verts, indices);
}

CRef<CMesh> CMeshBuilder::CreateSphere(
	IRenderer* pRenderer,
	CMaterial* pMaterial,
	float32 radius,
	int32 rings,
	int32 segments,
	const CColor& color,
	FaceOrientation_t::Enum faceOrientation)
{
	const uint32 packedColor = CColor::PackColor(color);

	CUtlVector<MeshBuilderVertex_t> verts;
	verts.SetCount((rings + 1) * (segments + 1));

	int32 vertexIndex = 0;
	for (int32 ring = 0; ring <= rings; ring++)
	{
		float32 theta = float32(ring) * CMaths::PI / float32(rings);
		float32 sinTheta = sinf(theta);
		float32 cosTheta = cosf(theta);
		for (int32 seg = 0; seg <= segments; seg++)
		{
			float32 phi = float32(seg) * CMaths::TwoPI / float32(segments);
			CVector3 pos(
				radius * sinTheta * cosf(phi),
				radius * cosTheta,
				radius * sinTheta * sinf(phi));

			CVector3 normal = pos;
			float32 len = normal.Length();
			if (len > 0.0f)
			{
				normal.m_X /= len;
				normal.m_Y /= len;
				normal.m_Z /= len;
			}

			if (faceOrientation == FaceOrientation_t::Inwards)
			{
				normal.m_X = -normal.m_X;
				normal.m_Y = -normal.m_Y;
				normal.m_Z = -normal.m_Z;
			}

			verts[vertexIndex] = MeshBuilderVertex_t(
				pos,
				normal,
				CVector2(0.0f, 0.0f),
				packedColor);

			vertexIndex++;
		}
	}

	CUtlVector<uint32> indices;
	indices.SetCount(rings * segments * 6);

	int32 indexPos = 0;
	for (int32 ring = 0; ring < rings; ring++)
	{
		for (int32 seg = 0; seg < segments; seg++)
		{
			int32 current = ring * (segments + 1) + seg;
			int32 next = current + segments + 1;

			if (faceOrientation == FaceOrientation_t::Inwards)
			{
				indices[indexPos++] = uint32(current);
				indices[indexPos++] = uint32(next);
				indices[indexPos++] = uint32(current + 1);
				indices[indexPos++] = uint32(current + 1);
				indices[indexPos++] = uint32(next);
				indices[indexPos++] = uint32(next + 1);
			}
			else
			{
				indices[indexPos++] = uint32(current);
				indices[indexPos++] = uint32(current + 1);
				indices[indexPos++] = uint32(next);
				indices[indexPos++] = uint32(current + 1);
				indices[indexPos++] = uint32(next + 1);
				indices[indexPos++] = uint32(next);
			}
		}
	}

	return CreateMesh(pRenderer, pMaterial, verts, indices);
}

CRef<CMesh> CMeshBuilder::CreatePlane(
	IRenderer* pRenderer,
	CMaterial* pMaterial,
	float32 halfSize,
	const CColor& color,
	FaceOrientation_t::Enum faceOrientation,
	int32 subdivisions)
{
	const uint32 packedColor = CColor::PackColor(color);

	CUtlVector<MeshBuilderVertex_t> verts;
	CUtlVector<uint32> indices;

	CreateFace(
		verts,
		indices,
		CVector3(-halfSize, 0.0f, halfSize),
		CVector3(halfSize, 0.0f, halfSize),
		CVector3(-halfSize, 0.0f, -halfSize),
		CVector3(halfSize, 0.0f, -halfSize),
		CVector3(0.0f, 1.0f, 0.0f),
		packedColor,
		subdivisions,
		faceOrientation);

	return CreateMesh(pRenderer, pMaterial, verts, indices);
}
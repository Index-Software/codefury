//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef RENDERITEM_H
#define RENDERITEM_H

#pragma once

#include "renderer/Renderer.h"

class CConstantBuffers;
class CMaterial;
struct CMatrix4;
class CMesh;
class CTransform;

struct RenderItem_t
{
	CMesh* m_pMesh;
	CTransform* m_pTransform;
	float32 m_SortKey;
	uint32 m_hEntity;
	int32 m_SubMeshIndex;

	RenderItem_t() :
		m_pMesh(CF_NULL),
		m_pTransform(CF_NULL),
		m_SortKey(0.0f),
		m_hEntity(0),
		m_SubMeshIndex(-1)
	{
	}

	RenderItem_t(
		uint32 hID,
		CMesh* pMesh,
		CTransform* pTransform,
		float32 key = 0.0f,
		int32 subIdx = -1) :
		m_pMesh(pMesh),
		m_pTransform(pTransform),
		m_SortKey(key),
		m_hEntity(hID),
		m_SubMeshIndex(subIdx)
	{
	}
};

void SetDepthStencilForQueue(
	IRenderer& renderer,
	int32 queueID,
	bool overrideDepthWrite = false);

CMaterial* GetMaterialForRenderItem(
	const RenderItem_t& renderItem,
	IRenderer& renderer);

void DrawRenderItem(
	const RenderItem_t& renderItem,
	IRenderer& renderer,
	CConstantBuffers& constantBuffers,
	const CMatrix4& viewProjection,
	CMaterial* pMaterial = CF_NULL,
	bool isAddPass = false);

#endif // RENDERITEM_H
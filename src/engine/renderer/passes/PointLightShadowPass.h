//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef POINTLIGHTSHADOWPASS_H
#define POINTLIGHTSHADOWPASS_H

#pragma once

#include "renderer/RenderQueue.h"
#include "Vector3.h"

class CPointLightShadowPass : public IRenderPass
{
public:
	CPointLightShadowPass();

	virtual void Render(
		RenderPassContext_t& context,
		CRenderQueueManager* pManager) CF_OVERRIDE;

private:
	struct ShadowCaster_t
	{
		CMatrix4 m_Model;
		CMesh* m_pMesh;
		int32 m_SubMeshIndex;

		ShadowCaster_t() :
			m_Model(1.0f),
			m_pMesh(CF_NULL),
			m_SubMeshIndex(-1)
		{
		}
	};

	bool UpdateCache(
		const RenderPassContext_t& context,
		const CVector3& lightPos);

	CUtlVector<ShadowCaster_t> m_Casters;
	CVector3 m_LightPos;
	RenderTargetHandle m_hRenderTarget;
	int32 m_RedrawFrames;
	bool m_IsCached;
};

#endif // POINTLIGHTSHADOWPASS_H
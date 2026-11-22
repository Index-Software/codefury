//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENVIRONMENTLIGHTING_H
#define ENVIRONMENTLIGHTING_H

#pragma once

#include "platform.h"
#include "mathsfury/Vector3.h"
#include "utils/RefCounted.h"
#include "shaders/ShaderConstants.h"

class IRenderer;
class CTexture;

class CEnvironmentLighting
{
public:
	CEnvironmentLighting();

	void Init(IRenderer& renderer);
	void Shutdown();

	bool Build(const CTexture& environmentMap);
	void Reset();
	void Bind() const;

	const CVector3* GetIrradianceSH() const
	{
		return m_IrradianceSH;
	}
private:
	bool LoadDFG();

	CVector3 m_IrradianceSH[CF_IBL_SH_COUNT];
	CRef<CTexture> m_pSpecularMap;
	CRef<CTexture> m_pDFGMap;
	IRenderer* m_pRenderer;
};

#endif // ENVIRONMENTLIGHTING_H
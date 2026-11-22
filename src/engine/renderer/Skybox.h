//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SKYBOX_H
#define SKYBOX_H

#pragma once

#include "dbg.h"
#include "WrenBind.h"
#include "renderer/Renderer.h"
#include "mathsfury/Maths.h"
#include "renderer/material/Material.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/Transform.h"
#include "utils/RefCounted.h"

class CSkybox : public CRefCounted
{
public:
	CSkybox(IRenderer* pRenderer) WREN_CONSTRUCTOR("init new()");

	bool Init() WREN_BIND("init");
	bool SetMaterial(CMaterial* pMaterial) WREN_BIND("setMaterial(_)");
	void SetPosition(const CVector3& position) WREN_BIND("setPosition(_)");

	CRef<CMesh> GetMesh() const WREN_BIND("getMesh")
	{
		return m_pMesh;
	}

	CRef<CMaterial> GetMaterial() const WREN_BIND("getMaterial")
	{
		return m_pMaterial;
	}

	CRef<CTransform> GetTransform() const WREN_BIND("getTransform")
	{
		return m_pTransform;
	}
private:
	IRenderer* m_pRenderer;
	CRef<CMesh> m_pMesh;
	CRef<CMaterial> m_pMaterial;
	CRef<CTransform> m_pTransform;
};

#endif // SKYBOX_H
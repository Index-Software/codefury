//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef MESHBUILDER_H
#define MESHBUILDER_H

#pragma once

#include "renderer/Renderer.h"
#include "utils/Color.h"
#include "WrenBind.h"
#include "ecs/components/Mesh.h"
#include "utils/RefCounted.h"

class CMaterial;

struct FaceOrientation_t
{
	enum Enum
	{
		Inwards,
		Outwards
	};
};

class CMeshBuilder
{
public:
	static CRef<CMesh> CreateCube(
		IRenderer* pRenderer,
		CMaterial* pMaterial,
		float32 halfSize,
		const CColor& color,
		FaceOrientation_t::Enum faceOrientation = FaceOrientation_t::Outwards,
		int32 subdivisions = 1) WREN_BIND_STATIC("createCube(_,_,_,_,_)");

	static CRef<CMesh> CreateSphere(
		IRenderer* pRenderer,
		CMaterial* pMaterial,
		float32 radius,
		int32 rings,
		int32 segments,
		const CColor& color,
		FaceOrientation_t::Enum faceOrientation = FaceOrientation_t::Outwards)
		WREN_BIND_STATIC("createSphere(_,_,_,_,_,_)");

	static CRef<CMesh> CreatePlane(
		IRenderer* pRenderer,
		CMaterial* pMaterial,
		float32 halfSize,
		const CColor& color,
		FaceOrientation_t::Enum faceOrientation = FaceOrientation_t::Outwards,
		int32 subdivisions = 1) WREN_BIND_STATIC("createPlane(_,_,_,_,_)");
};

#endif // MESHBUILDER_H
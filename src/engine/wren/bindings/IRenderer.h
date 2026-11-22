//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef IRENDERER_H
#define IRENDERER_H

#pragma once

#ifdef WREN_BIND_CODEGEN

#include "platform.h"
#include "WrenBind.h"

struct CColor;

struct IRenderer : public CWrenBindSingleton<IRenderer>
{
	WREN_SINGLETON_DECL()

	void BeginFrame() WREN_BIND_SINGLETON("beginFrame()", s_pInstance);
	void EndFrame() WREN_BIND_SINGLETON("endFrame()", s_pInstance);
	void Clear(
		uint32,
		const CColor&,
		float32,
		uint32) WREN_BIND_SINGLETON("clear(_,_,_,_)", s_pInstance);
	void SetFullViewport()
		WREN_BIND_SINGLETON("setFullViewport()", s_pInstance);
	float32 GetAspectRatio() const
		WREN_BIND_SINGLETON("getAspectRatio()", s_pInstance);
};

#endif // WREN_BIND_CODEGEN

#endif // IRENDERER_H
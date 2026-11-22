//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef TEXTURE_H
#define TEXTURE_H

#pragma once

#include "renderer/Renderer.h"
#include "WrenBind.h"
#include "utils/RefCounted.h"

class CTexture : public CRefCounted
{
public:
	CTexture(IRenderer* pRenderer) WREN_CONSTRUCTOR("init new()");
	virtual ~CTexture() CF_OVERRIDE;

	bool LoadFromFile(const char* pFilename) WREN_BIND("loadFromFile(_)");
	bool CreateFromMemory(
		uint32 width,
		uint32 height,
		const CUtlVector<uint8>& pixelData)
		WREN_BIND("createFromMemory(_,_,_)");
	bool CreateFromCubeMemory(
		uint32 size,
		const CUtlVector< CUtlVector<uint8> >& faces)
		WREN_BIND("createFromCubeMemory(_,_)");
	bool CreateFromCubeMipMemory(
		uint32 size,
		uint32 mipCount,
		TextureFormat_t::Enum format,
		const CUtlVector< CUtlVector<uint8> >& levelFaces);
	bool CreateCubemapFromEquirect(const char* pFilename, int32 faceSize)
		WREN_BIND("createCubemapFromEquirect(_,_)");

	void Bind(uint32 slot) const;

	void SetWrapMode(TextureWrapMode_t::Enum mode) WREN_BIND("setWrapMode(_)")
	{
		m_WrapMode = mode;
	}

	TextureWrapMode_t::Enum GetWrapMode() const WREN_BIND("getWrapMode")
	{
		return m_WrapMode;
	}

	TextureHandle GetHandle() const WREN_BIND("getHandle")
	{
		return m_hTexture;
	}

	uint32 GetWidth() const WREN_BIND("getWidth")
	{
		return m_Width;
	}

	uint32 GetHeight() const WREN_BIND("getHeight")
	{
		return m_Height;
	}

	const CUtlVector<uint8>& GetData() const WREN_BIND("getData")
	{
		return m_Data;
	}

	const CUtlVector< CUtlVector<uint8> >& GetCubeFaces() const
	{
		return m_CubeFaces;
	}

	const CFixedString& GetSourcePath() const
	{
		return m_SourcePath;
	}
private:
	CFixedString m_SourcePath;
	CUtlVector<uint8> m_Data;
	CUtlVector< CUtlVector<uint8> > m_CubeFaces;
	IRenderer* m_pRenderer;
	TextureHandle m_hTexture;
	uint32 m_Width;
	uint32 m_Height;
	TextureWrapMode_t::Enum m_WrapMode;
};

#endif // TEXTURE_H
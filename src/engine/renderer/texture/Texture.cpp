//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/texture/Texture.h"
#include "dbg.h"
#include "renderer/ibl/EnvironmentFaces.h"
#include "utils/Utils.h"
#include "mathsfury/Maths.h"
#include <string.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

CTexture::CTexture(IRenderer* pRenderer)
	: m_pRenderer(pRenderer)
	, m_hTexture(0)
	, m_Width(0)
	, m_Height(0)
	, m_AnisoLevel(1)
	, m_WrapMode(TextureWrapMode_t::Repeat)
	, m_IsMipmapEnabled(false)
{
}

CTexture::~CTexture()
{
	if (m_hTexture != 0)
	{
		m_pRenderer->DestroyTexture(m_hTexture);
	}
}

bool CTexture::LoadFromFile(const char* pFilename)
{
	const CFixedString fullPath = CUtils::GetFullPath(pFilename);

	int32 width;
	int32 height;
	int32 channels;
	uint8* pImageData = reinterpret_cast<uint8*>(stbi_load(
		fullPath.c_str(),
		&width,
		&height,
		&channels,
		STBI_rgb_alpha));

	if (!pImageData)
	{
		Error("[CTexture] Failed to load image %s\n", fullPath.c_str());

		return false;
	}

	m_Width = uint32(width);
	m_Height = uint32(height);

	uint64 dataSize = uint64(width) * uint64(height) * 4;
	m_Data.SetCount(int32(dataSize));
	if (m_Data.Base() == CF_NULL)
	{
		Error("[CTexture] Failed to allocate m_Data buffer!\n");

		stbi_image_free(pImageData);

		return false;
	}

	memcpy(m_Data.Base(), pImageData, uint64(dataSize));

	stbi_image_free(pImageData);

	m_hTexture = m_pRenderer->CreateTexture2D(
		m_Width,
		m_Height,
		TextureFormat_t::RGBA8,
		m_Data.Base(),
		m_IsMipmapEnabled);

	if (m_hTexture == 0)
	{
		Error("[CTexture] Failed to create texture %s\n", fullPath.c_str());

		return false;
	}

	m_pRenderer->SetTextureAnisoLevel(m_hTexture, m_AnisoLevel);

	return true;
}

bool CTexture::CreateFromMemory(
	uint32 width,
	uint32 height,
	const CUtlVector<uint8>& pixelData)
{
	if (pixelData.Count() == 0)
	{
		return false;
	}

	m_Width = width;
	m_Height = height;
	m_Data = pixelData;
	m_hTexture = m_pRenderer->CreateTexture2D(
		m_Width,
		m_Height,
		TextureFormat_t::RGBA8,
		m_Data.Base(),
		m_IsMipmapEnabled);
	if (m_hTexture == 0)
	{
		Error("[CTexture] Failed to create texture from memory\n");

		return false;
	}

	m_pRenderer->SetTextureAnisoLevel(m_hTexture, m_AnisoLevel);

	return true;
}

bool CTexture::CreateFromCubeMemory(
	uint32 size,
	const CUtlVector< CUtlVector<uint8> >& faces)
{
	m_WrapMode = TextureWrapMode_t::ClampToEdge;

	if (faces.Count() != 6)
	{
		return false;
	}

	m_Width = size;
	m_Height = size;
	const void* pFacePtrs[6];
	for (int32 i = 0; i < 6; i++)
	{
		if (faces[i].Count() == 0)
		{
			return false;
		}

		pFacePtrs[i] = faces[i].Base();
	}

	m_hTexture = m_pRenderer->CreateTextureCube(
		size,
		TextureFormat_t::RGBA8,
		pFacePtrs);
	if (m_hTexture == 0)
	{
		Error("[CTexture] Failed to create cube texture from memory\n");

		return false;
	}

	m_pRenderer->SetTextureAnisoLevel(m_hTexture, m_AnisoLevel);

	m_CubeFaces = faces;

	return true;
}

bool CTexture::CreateFromCubeMipMemory(
	uint32 size,
	uint32 mipCount,
	TextureFormat_t::Enum format,
	const CUtlVector< CUtlVector<uint8> >& levelFaces)
{
	m_WrapMode = TextureWrapMode_t::ClampToEdge;

	if (levelFaces.Count() != int32(6 * mipCount))
	{
		return false;
	}

	CUtlVector<const void*> facePtrs;
	facePtrs.SetCount(levelFaces.Count());
	for (int32 i = 0; i < levelFaces.Count(); i++)
	{
		if (levelFaces[i].Count() == 0)
		{
			return false;
		}

		facePtrs[i] = levelFaces[i].Base();
	}

	m_Width = size;
	m_Height = size;
	m_hTexture = m_pRenderer->CreateTextureCube(
		size,
		format,
		facePtrs.Base(),
		mipCount);
	if (m_hTexture == 0)
	{
		Error("[CTexture] Failed to create mipmapped cube texture\n");

		return false;
	}

	m_pRenderer->SetTextureAnisoLevel(m_hTexture, m_AnisoLevel);

	return true;
}

bool CTexture::CreateCubemapFromEquirect(const char* pFilename, int32 faceSize)
{
	m_WrapMode = TextureWrapMode_t::ClampToEdge;

	const CFixedString fullPath = CUtils::GetFullPath(pFilename);

	int32 width, height, channels;
	uint8* pData = reinterpret_cast<uint8*>(
		stbi_load(fullPath.c_str(), &width, &height, &channels, 4));
	if (!pData)
	{
		Error(
			"[CTexture] Failed to load equirectangular texture: %s\n",
			fullPath.c_str());

		return false;
	}

	CUtlVector<uint8> pixels;
	pixels.SetCount(width * height * 4);
	memcpy(pixels.Base(), pData, pixels.Count());
	stbi_image_free(pData);

	Msg(
		"Converting equirectangular to cubemap (%dx%d)...\n",
		faceSize,
		faceSize);

	CUtlVector< CUtlVector<uint8> > faces;
	CEnvironmentFaces::ConvertEquirectToFaces(
		pixels, width, height, faceSize, faces);
	if (!CreateFromCubeMemory(uint32(faceSize), faces))
	{
		return false;
	}

	m_SourcePath = pFilename;

	return true;
}

void CTexture::Bind(uint32 slot) const
{
	m_pRenderer->SetTexture(m_hTexture, slot, ShaderStageFragment, m_WrapMode);
}

void CTexture::SetAnisoLevel(uint32 anisoLevel)
{
	m_AnisoLevel = anisoLevel > 0 ? anisoLevel : 1;
	if (m_hTexture != 0)
	{
		m_pRenderer->SetTextureAnisoLevel(m_hTexture, m_AnisoLevel);
	}
}
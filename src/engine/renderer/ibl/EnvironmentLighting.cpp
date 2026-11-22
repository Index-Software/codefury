//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/ibl/EnvironmentLighting.h"
#include "renderer/ibl/EnvironmentPrefilter.h"
#include "renderer/ibl/EnvironmentData.h"
#include "renderer/ibl/EnvironmentSerializer.h"
#include "renderer/texture/Texture.h"
#include "utils/Utils.h"
#include "shaders/ShaderBindings.h"
#include "dbg.h"
#include <stdio.h>
#include <string.h>

static const char* s_pDFGPath = "assets/textures/ibl/dfg.png";
static const float32 s_DefaultAmbientSH[3] = { 0.08f, 0.08f, 0.1f };

static bool GetPrefilteredPath(
	const char* pSourcePath,
	int32 faceSize,
	char* pOutPath,
	uint32 pathSize)
{
	const char* pExtension = strrchr(pSourcePath, '.');
	const char* pSlash = strrchr(pSourcePath, '/');
	const char* pBackslash = strrchr(pSourcePath, '\\');
	const char* pSeparator = pSlash && pBackslash
		? (pSlash > pBackslash ? pSlash : pBackslash)
		: (pSlash ? pSlash : pBackslash);
	const bool hasExtension =
		pExtension && (!pSeparator || pExtension > pSeparator);
	const int32 stemLength = hasExtension
		? int32(pExtension - pSourcePath)
		: int32(strlen(pSourcePath));

	const int32 length = snprintf(
		pOutPath,
		pathSize,
		"%.*s.%d.ibl",
		stemLength,
		pSourcePath,
		faceSize);

	return length > 0 && uint32(length) < pathSize;
}

static bool LoadPrefiltered(
	const CTexture& environmentMap,
	EnvironmentData_t& outData)
{
	const CFixedString& sourcePath = environmentMap.GetSourcePath();
	if (sourcePath.IsEmpty())
	{
		return false;
	}

	char prefilteredPath[512];
	if (!GetPrefilteredPath(
		sourcePath.c_str(),
		int32(environmentMap.GetWidth()),
		prefilteredPath,
		sizeof(prefilteredPath)))
	{
		return false;
	}

	const CUtlVector<uint8> bytes = CUtils::ReadBinaryFile(prefilteredPath);
	if (bytes.Count() == 0)
	{
		Warning(
			"[CEnvironmentLighting] Empty prefiltered environment: %s\n",
			prefilteredPath);

		return false;
	}

	if (!CEnvironmentSerializer::Deserialize(bytes, outData))
	{
		Warning(
			"[CEnvironmentLighting] Invalid prefiltered environment: %s\n",
			prefilteredPath);

		return false;
	}

	Msg("Loaded prefiltered environment %s\n", prefilteredPath);

	return true;
}

static bool PrefilterEnvironment(
	const CTexture& environmentMap,
	EnvironmentData_t& outData)
{
	const CUtlVector< CUtlVector<uint8> >& faces =
		environmentMap.GetCubeFaces();
	if (faces.Count() != 6)
	{
		Warning("[CEnvironmentLighting] Environment map is not a cubemap\n");

		return false;
	}

	Warning(
		"[CEnvironmentLighting] No usable cached prefiltered "
		"environment; falling back to runtime prefiltering\n");
	if (!CEnvironmentPrefilter::Prefilter(
		faces,
		int32(environmentMap.GetWidth()),
		outData))
	{
		Warning(
			"[CEnvironmentLighting] Failed to prefilter the environment map\n");

		return false;
	}

	return true;
}

static bool CreateSpecularMap(
	IRenderer& renderer,
	const EnvironmentData_t& data,
	CRef<CTexture>& outMap)
{
	if (data.m_SpecularMipCount == 0)
	{
		Warning(
			"[CEnvironmentLighting] Environment map is not a power "
			"of two\n");

		return false;
	}

	outMap = CRef<CTexture>::Create(&renderer);
	if (!outMap ||
		!outMap->CreateFromCubeMipMemory(
			uint32(data.m_SpecularSize),
			uint32(data.m_SpecularMipCount),
			TextureFormat_t::SRGBA8,
			data.m_SpecularFaces))
	{
		Warning("[CEnvironmentLighting] Failed to create the specular map\n");
		outMap.Reset();

		return false;
	}

	return true;
}

CEnvironmentLighting::CEnvironmentLighting() :
	m_pRenderer(CF_NULL)
{
	Reset();
}

void CEnvironmentLighting::Init(IRenderer& renderer)
{
	m_pRenderer = &renderer;
	LoadDFG();
}

void CEnvironmentLighting::Shutdown()
{
	m_pSpecularMap.Reset();
	m_pDFGMap.Reset();
	m_pRenderer = CF_NULL;
}

bool CEnvironmentLighting::Build(const CTexture& environmentMap)
{
	EnvironmentData_t data;
	if (!LoadPrefiltered(environmentMap, data) &&
		!PrefilterEnvironment(environmentMap, data))
	{
		return false;
	}

	CRef<CTexture> specularMap;
	if (!CreateSpecularMap(*m_pRenderer, data, specularMap))
	{
		return false;
	}

	for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
	{
		m_IrradianceSH[i] = data.m_IrradianceSH[i];
	}
	m_pSpecularMap = specularMap;

	return true;
}

void CEnvironmentLighting::Reset()
{
	m_IrradianceSH[0] = CVector3(
		s_DefaultAmbientSH[0],
		s_DefaultAmbientSH[1],
		s_DefaultAmbientSH[2]);
	for (int32 i = 1; i < CF_IBL_SH_COUNT; i++)
	{
		m_IrradianceSH[i] = CVector3(0.0f, 0.0f, 0.0f);
	}

	m_pSpecularMap.Reset();
}

void CEnvironmentLighting::Bind() const
{
	if (m_pSpecularMap)
	{
		m_pSpecularMap->Bind(CF_BINDING_IBL_SPECULAR);
	}

	if (m_pDFGMap)
	{
		m_pDFGMap->Bind(CF_BINDING_IBL_DFG);
	}
}

bool CEnvironmentLighting::LoadDFG()
{
	m_pDFGMap = CRef<CTexture>::Create(m_pRenderer);
	if (!m_pDFGMap || !m_pDFGMap->LoadFromFile(s_pDFGPath))
	{
		Warning(
			"[CEnvironmentLighting] Failed to load the DFG map %s\n",
			s_pDFGPath);
		m_pDFGMap.Reset();

		return false;
	}

	m_pDFGMap->SetWrapMode(TextureWrapMode_t::ClampToEdge);

	return true;
}

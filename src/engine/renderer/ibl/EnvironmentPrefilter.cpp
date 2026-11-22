//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/ibl/EnvironmentPrefilter.h"
#include "renderer/ibl/EnvironmentData.h"
#include "renderer/ibl/EnvironmentFaces.h"
#include "renderer/ibl/CubemapSH.h"
#include "renderer/ibl/CubemapIBL.h"
#include "mathsfury/Maths.h"
#include <math.h>

static const int32 s_SpecularSampleCount = 64;

static float32 GetLinearRoughnessForMip(int32 mip)
{
	if (mip == 0)
	{
		return 0.0f;
	}

	const float32 lod = float32(mip) / float32(CF_IBL_SPECULAR_LOD_STEPS);
	const float32 perceptualRoughness =
		(1.7f - sqrtf(2.89f - 2.8f * lod)) / 1.4f;

	return perceptualRoughness * perceptualRoughness;
}

bool CEnvironmentPrefilter::Prefilter(
	const CUtlVector< CUtlVector<uint8> >& faces,
	int32 faceSize,
	EnvironmentData_t& outData)
{
	if (faces.Count() != 6 ||
		faceSize <= 0 ||
		!CMaths::IsPowerOfTwo(uint32(faceSize)))
	{
		return false;
	}

	for (int32 i = 0; i < faces.Count(); i++)
	{
		if (faces[i].Count() < faceSize * faceSize * 4)
		{
			return false;
		}
	}

	CubemapSH::computeSH(faces, faceSize, 3, true, outData.m_IrradianceSH);
	CubemapSH::preprocessSHForShader(outData.m_IrradianceSH);

	outData.m_SpecularFaces.RemoveAll();
	outData.m_SpecularSize = 0;
	outData.m_SpecularMipCount = 0;

	CUtlVector<CEnvironmentFaces> sourceLevels;
	sourceLevels.SetCount(int32(CMaths::Log2(uint32(faceSize))) + 1);
	sourceLevels[0].InitFromFaces(faces, faceSize);
	for (int32 i = 1; i < sourceLevels.Count(); i++)
	{
		sourceLevels[i - 1].Downsample(sourceLevels[i]);
	}

	const int32 size = CMaths::Min(faceSize, s_EnvironmentSpecularMaxSize);
	const int32 mipCount = CMaths::Min(
		CF_IBL_SPECULAR_LOD_STEPS + 1,
		int32(CMaths::Log2(uint32(size))) + 1);

	outData.m_SpecularSize = size;
	outData.m_SpecularMipCount = mipCount;
	outData.m_SpecularFaces.SetCount(6 * mipCount);
	CEnvironmentFaces level;
	for (int32 mip = 0; mip < mipCount; mip++)
	{
		level.Init(size >> mip);
		CubemapIBL::roughnessFilter(
			sourceLevels,
			GetLinearRoughnessForMip(mip),
			s_SpecularSampleCount,
			level);
		for (int32 face = 0; face < 6; face++)
		{
			level.WriteFace(face, outData.m_SpecularFaces[mip * 6 + face]);
		}
	}

	return true;
}
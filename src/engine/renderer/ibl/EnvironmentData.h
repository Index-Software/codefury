//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENVIRONMENTDATA_H
#define ENVIRONMENTDATA_H

#pragma once

#include "platform.h"
#include "utils/UtlVector.h"
#include "mathsfury/Vector3.h"
#include "shaders/ShaderConstants.h"

static const int32 s_EnvironmentSpecularMaxSize = 128;

struct EnvironmentData_t
{
	CVector3 m_IrradianceSH[CF_IBL_SH_COUNT];
	CUtlVector< CUtlVector<uint8> > m_SpecularFaces;
	int32 m_SpecularSize;
	int32 m_SpecularMipCount;

	EnvironmentData_t() :
		m_SpecularSize(0),
		m_SpecularMipCount(0)
	{
		for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
		{
			m_IrradianceSH[i] = CVector3(0.0f, 0.0f, 0.0f);
		}
	}

	int32 GetFacesBytes() const
	{
		int32 bytes = 0;
		for (int32 mip = 0; mip < m_SpecularMipCount; mip++)
		{
			const int32 levelSize = m_SpecularSize >> mip;
			bytes += 6 * levelSize * levelSize * 4;
		}

		return bytes;
	}
};

#endif // ENVIRONMENTDATA_H
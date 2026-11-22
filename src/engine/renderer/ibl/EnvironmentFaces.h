//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef ENVIRONMENTFACES_H
#define ENVIRONMENTFACES_H

#pragma once

#include "platform.h"
#include "utils/UtlVector.h"
#include "mathsfury/Vector3.h"

class CEnvironmentFaces
{
public:
	CEnvironmentFaces();

	static void ConvertEquirectToFaces(
		const CUtlVector<uint8>& pixels,
		int32 width,
		int32 height,
		int32 faceSize,
		CUtlVector< CUtlVector<uint8> >& outFaces);

	void Init(int32 size);
	void InitFromFaces(
		const CUtlVector< CUtlVector<uint8> >& faces,
		int32 size);

	void Downsample(CEnvironmentFaces& outFaces) const;
	void WriteFace(int32 face, CUtlVector<uint8>& outPixels) const;
	CVector3 SampleBilinear(const CVector3& direction) const;

	const CVector3& GetTexel(int32 face, int32 x, int32 y) const
	{
		return m_Texels[(face * m_Size + y) * m_Size + x];
	}

	void SetTexel(int32 face, int32 x, int32 y, const CVector3& color)
	{
		m_Texels[(face * m_Size + y) * m_Size + x] = color;
	}

	int32 GetSize() const
	{
		return m_Size;
	}
private:
	CUtlVector<CVector3> m_Texels;
	int32 m_Size;
};

#endif // ENVIRONMENTFACES_H
//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/ibl/EnvironmentFaces.h"
#include "renderer/ibl/Cubemap.h"
#include "renderer/texture/ColorSpace.h"
#include "mathsfury/Maths.h"
#include <math.h>

static float32 DecodeSRGB(uint8 value)
{
	return CColorSpace::SRGBToLinear(float32(value) / 255.0f);
}

static uint8 EncodeSRGB(float32 value)
{
	const float32 clamped = CMaths::Clamp(value, 0.0f, 1.0f);

	return uint8(CColorSpace::LinearToSRGB(clamped) * 255.0f + 0.5f);
}

static void SampleEquirect(
	const CUtlVector<uint8>& pixels,
	int32 width,
	int32 height,
	float32 u,
	float32 v,
	uint8* pOut)
{
	const float32 wrappedU = u < 0.0f ? u + 1.0f : (u > 1.0f ? u - 1.0f : u);
	const float32 sampleX = wrappedU * float32(width - 1);
	const float32 sampleY = CMaths::Clamp(v, 0.0f, 1.0f) * float32(height - 1);
	const int32 x0 = int32(sampleX);
	const int32 y0 = int32(sampleY);
	const int32 x1 = CMaths::Min(x0 + 1, width - 1);
	const int32 y1 = CMaths::Min(y0 + 1, height - 1);
	const float32 weightX = sampleX - float32(x0);
	const float32 weightY = sampleY - float32(y0);
	const float32 weight00 = (1.0f - weightX) * (1.0f - weightY);
	const float32 weight10 = weightX * (1.0f - weightY);
	const float32 weight01 = (1.0f - weightX) * weightY;
	const float32 weight11 = weightX * weightY;
	for (int32 i = 0; i < 4; i++)
	{
		const float32 value =
			pixels[(y0 * width + x0) * 4 + i] * weight00 +
			pixels[(y0 * width + x1) * 4 + i] * weight10 +
			pixels[(y1 * width + x0) * 4 + i] * weight01 +
			pixels[(y1 * width + x1) * 4 + i] * weight11;

		pOut[i] = uint8(CMaths::Clamp(value, 0.0f, 255.0f));
	}
}

CEnvironmentFaces::CEnvironmentFaces() :
	m_Size(0)
{
}

void CEnvironmentFaces::ConvertEquirectToFaces(
	const CUtlVector<uint8>& pixels,
	int32 width,
	int32 height,
	int32 faceSize,
	CUtlVector< CUtlVector<uint8> >& outFaces)
{
	const Cubemap cubemap(faceSize);
	outFaces.SetCount(6);
	for (int32 face = 0; face < 6; face++)
	{
		outFaces[face].SetCount(faceSize * faceSize * 4);
		uint8* pDst = outFaces[face].Base();
		for (int32 y = 0; y < faceSize; y++)
		{
			for (int32 x = 0; x < faceSize; x++, pDst += 4)
			{
				const CVector3 direction = cubemap.getDirectionFor(
					static_cast<Cubemap::Face::Enum>(face),
					float32(x) + 0.5f,
					float32(y) + 0.5f);
				const float32 equirectU = 0.5f +
					atan2f(direction.m_Z, direction.m_X) / (2.0f * CMaths::PI);
				const float32 equirectV =
					0.5f - asinf(direction.m_Y) / CMaths::PI;

				SampleEquirect(
					pixels,
					width,
					height,
					equirectU,
					equirectV,
					pDst);
			}
		}
	}
}

void CEnvironmentFaces::Init(int32 size)
{
	m_Size = size;
	m_Texels.SetCount(6 * size * size);
	for (int32 i = 0; i < m_Texels.Count(); i++)
	{
		m_Texels[i] = CVector3(0.0f, 0.0f, 0.0f);
	}
}

void CEnvironmentFaces::InitFromFaces(
	const CUtlVector< CUtlVector<uint8> >& faces,
	int32 size)
{
	Init(size);
	for (int32 face = 0; face < 6; face++)
	{
		const uint8* pSrc = faces[face].Base();
		for (int32 y = 0; y < size; y++)
		{
			for (int32 x = 0; x < size; x++, pSrc += 4)
			{
				SetTexel(
					face,
					x,
					y,
					CVector3(
						DecodeSRGB(pSrc[0]),
						DecodeSRGB(pSrc[1]),
						DecodeSRGB(pSrc[2])));
			}
		}
	}
}

void CEnvironmentFaces::Downsample(CEnvironmentFaces& outFaces) const
{
	const int32 size = m_Size > 1 ? m_Size / 2 : 1;
	outFaces.Init(size);
	for (int32 face = 0; face < 6; face++)
	{
		for (int32 y = 0; y < size; y++)
		{
			for (int32 x = 0; x < size; x++)
			{
				const int32 x0 = m_Size > 1 ? x * 2 : 0;
				const int32 y0 = m_Size > 1 ? y * 2 : 0;
				const int32 x1 = m_Size > 1 ? x0 + 1 : 0;
				const int32 y1 = m_Size > 1 ? y0 + 1 : 0;
				const CVector3 sum =
					GetTexel(face, x0, y0) +
					GetTexel(face, x1, y0) +
					GetTexel(face, x0, y1) +
					GetTexel(face, x1, y1);

				outFaces.SetTexel(face, x, y, sum * 0.25f);
			}
		}
	}
}

void CEnvironmentFaces::WriteFace(
	int32 face,
	CUtlVector<uint8>& outPixels) const
{
	outPixels.SetCount(m_Size * m_Size * 4);
	uint8* pDst = outPixels.Base();
	for (int32 y = 0; y < m_Size; y++)
	{
		for (int32 x = 0; x < m_Size; x++, pDst += 4)
		{
			const CVector3& color = GetTexel(face, x, y);
			pDst[0] = EncodeSRGB(color.m_X);
			pDst[1] = EncodeSRGB(color.m_Y);
			pDst[2] = EncodeSRGB(color.m_Z);
			pDst[3] = 255;
		}
	}
}

CVector3 CEnvironmentFaces::SampleBilinear(const CVector3& direction) const
{
	const Cubemap::Address address = Cubemap::getAddressFor(direction);
	const int32 face = int32(address.face);
	const float32 maxCoord = float32(m_Size - 1);
	const float32 x = CMaths::Clamp(
		address.s * float32(m_Size) - 0.5f,
		0.0f,
		maxCoord);
	const float32 y = CMaths::Clamp(
		address.t * float32(m_Size) - 0.5f,
		0.0f,
		maxCoord);

	const int32 x0 = int32(x);
	const int32 y0 = int32(y);
	const int32 x1 = x0 + 1 < m_Size ? x0 + 1 : x0;
	const int32 y1 = y0 + 1 < m_Size ? y0 + 1 : y0;
	const float32 fracX = x - float32(x0);
	const float32 fracY = y - float32(y0);
	const CVector3 top = GetTexel(face, x0, y0) * (1.0f - fracX) +
		GetTexel(face, x1, y0) * fracX;
	const CVector3 bottom = GetTexel(face, x0, y1) * (1.0f - fracX) +
		GetTexel(face, x1, y1) * fracX;

	return top * (1.0f - fracY) + bottom * fracY;
}
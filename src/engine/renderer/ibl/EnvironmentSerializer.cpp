//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/ibl/EnvironmentSerializer.h"
#include "renderer/ibl/EnvironmentData.h"
#include "mathsfury/Maths.h"
#include <string.h>

static const uint32 s_FileMagic =
	uint32('C') |
	(uint32('F') << 8) |
	(uint32('I') << 16) |
	(uint32('B') << 24);
static const uint32 s_FileVersion = 1;
static const int32 s_FileHeaderSize = 4 * 4 + CF_IBL_SH_COUNT * 3 * 4;

struct ByteReader_t
{
	const uint8* m_pData;
	int32 m_Size;
	int32 m_Offset;

	bool ReadUint32(uint32& outValue)
	{
		if (m_Offset + 4 > m_Size)
		{
			return false;
		}

		const uint8* pSrc = m_pData + m_Offset;
		outValue = uint32(pSrc[0]) |
			(uint32(pSrc[1]) << 8) |
			(uint32(pSrc[2]) << 16) |
			(uint32(pSrc[3]) << 24);
		m_Offset += 4;

		return true;
	}

	bool ReadFloat32(float32& outValue)
	{
		uint32 bits;
		if (!ReadUint32(bits))
		{
			return false;
		}

		memcpy(&outValue, &bits, sizeof(outValue));

		return true;
	}
};

struct ByteWriter_t
{
	uint8* m_pData;
	int32 m_Offset;

	void WriteUint32(uint32 value)
	{
		m_pData[m_Offset] = uint8(value);
		m_pData[m_Offset + 1] = uint8(value >> 8);
		m_pData[m_Offset + 2] = uint8(value >> 16);
		m_pData[m_Offset + 3] = uint8(value >> 24);
		m_Offset += 4;
	}

	void WriteFloat32(float32 value)
	{
		uint32 bits;
		memcpy(&bits, &value, sizeof(bits));
		WriteUint32(bits);
	}
};

static bool IsSerializable(const EnvironmentData_t& data)
{
	if (data.m_SpecularSize <= 0 ||
		data.m_SpecularSize > s_EnvironmentSpecularMaxSize ||
		data.m_SpecularMipCount <= 0 ||
		data.m_SpecularMipCount > CMaths::Log2(data.m_SpecularSize) + 1 ||
		data.m_SpecularFaces.Count() != 6 * data.m_SpecularMipCount)
	{
		return false;
	}

	for (int32 mip = 0; mip < data.m_SpecularMipCount; mip++)
	{
		const int32 levelSize = data.m_SpecularSize >> mip;
		const int32 faceBytes = levelSize * levelSize * 4;
		for (int32 face = 0; face < 6; face++)
		{
			if (data.m_SpecularFaces[mip * 6 + face].Count() != faceBytes)
			{
				return false;
			}
		}
	}

	return true;
}

bool CEnvironmentSerializer::Serialize(
	const EnvironmentData_t& data,
	CUtlVector<uint8>& outBytes)
{
	if (!IsSerializable(data))
	{
		return false;
	}

	outBytes.SetCount(s_FileHeaderSize + data.GetFacesBytes());

	ByteWriter_t writer = { outBytes.Base(), 0 };
	writer.WriteUint32(s_FileMagic);
	writer.WriteUint32(s_FileVersion);
	writer.WriteUint32(uint32(data.m_SpecularSize));
	writer.WriteUint32(uint32(data.m_SpecularMipCount));

	for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
	{
		writer.WriteFloat32(data.m_IrradianceSH[i].m_X);
		writer.WriteFloat32(data.m_IrradianceSH[i].m_Y);
		writer.WriteFloat32(data.m_IrradianceSH[i].m_Z);
	}

	for (int32 i = 0; i < data.m_SpecularFaces.Count(); i++)
	{
		const CUtlVector<uint8>& pixels = data.m_SpecularFaces[i];
		memcpy(
			writer.m_pData + writer.m_Offset,
			pixels.Base(),
			size_t(pixels.Count()));
		writer.m_Offset += pixels.Count();
	}

	return true;
}

bool CEnvironmentSerializer::Deserialize(
	const CUtlVector<uint8>& bytes,
	EnvironmentData_t& outData)
{
	ByteReader_t reader = { bytes.Base(), bytes.Count(), 0 };

	uint32 magic;
	uint32 version;
	uint32 size;
	uint32 mipCount;
	if (!reader.ReadUint32(magic) ||
		!reader.ReadUint32(version) ||
		!reader.ReadUint32(size) ||
		!reader.ReadUint32(mipCount))
	{
		return false;
	}

	if (magic != s_FileMagic || version != s_FileVersion)
	{
		return false;
	}

	if (size == 0 || size > uint32(s_EnvironmentSpecularMaxSize) ||
		mipCount == 0 || mipCount > CMaths::Log2(size) + 1 ||
		!CMaths::IsPowerOfTwo(size))
	{
		return false;
	}

	EnvironmentData_t parsed;
	for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
	{
		CVector3& coefficient = parsed.m_IrradianceSH[i];
		if (!reader.ReadFloat32(coefficient.m_X) ||
			!reader.ReadFloat32(coefficient.m_Y) ||
			!reader.ReadFloat32(coefficient.m_Z))
		{
			return false;
		}
	}

	parsed.m_SpecularSize = int32(size);
	parsed.m_SpecularMipCount = int32(mipCount);
	if (reader.m_Offset + parsed.GetFacesBytes() != reader.m_Size)
	{
		return false;
	}

	parsed.m_SpecularFaces.SetCount(6 * parsed.m_SpecularMipCount);
	for (int32 mip = 0; mip < parsed.m_SpecularMipCount; mip++)
	{
		const int32 levelSize = parsed.m_SpecularSize >> mip;
		const int32 faceBytes = levelSize * levelSize * 4;
		for (int32 face = 0; face < 6; face++)
		{
			CUtlVector<uint8>& pixels = parsed.m_SpecularFaces[mip * 6 + face];
			pixels.SetCount(faceBytes);
			memcpy(
				pixels.Base(),
				reader.m_pData + reader.m_Offset,
				size_t(faceBytes));
			reader.m_Offset += faceBytes;
		}
	}

	outData = parsed;

	return true;
}
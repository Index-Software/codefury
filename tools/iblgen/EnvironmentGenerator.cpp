//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "EnvironmentGenerator.h"
#include "dbg.h"
#include "renderer/ibl/CubemapIBL.h"
#include "renderer/ibl/EnvironmentData.h"
#include "renderer/ibl/EnvironmentFaces.h"
#include "renderer/ibl/EnvironmentPrefilter.h"
#include "renderer/ibl/EnvironmentSerializer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

static const int32 s_DFGSampleCount = 1024;

static bool LoadImage(
	const char* pPath,
	CUtlVector<uint8>& outPixels,
	int32& outWidth,
	int32& outHeight)
{
	int32 channels;
	uint8* pData = stbi_load(pPath, &outWidth, &outHeight, &channels, 4);
	if (!pData)
	{
		return false;
	}

	outPixels.SetCount(outWidth * outHeight * 4);
	memcpy(outPixels.Base(), pData, size_t(outPixels.Count()));
	stbi_image_free(pData);

	return true;
}

static bool WriteFile(
	const char* pPath,
	const CUtlVector<uint8>& bytes)
{
	FILE* pFile = fopen(pPath, "wb");
	if (!pFile)
	{
		return false;
	}

	const size_t written = fwrite(
		bytes.Base(),
		1,
		size_t(bytes.Count()),
		pFile);
	fclose(pFile);

	return written == size_t(bytes.Count());
}

bool CEnvironmentGenerator::Generate(
	const char* pSourcePath,
	const char* pFaceSize,
	const char* pOutputPath)
{
	const int32 faceSize = int32(atoi(pFaceSize));
	if (faceSize <= 0)
	{
		Error("[CEnvironmentGenerator] Invalid face size: %s\n", pFaceSize);

		return false;
	}

	CUtlVector<uint8> pixels;
	int32 width;
	int32 height;
	if (!LoadImage(pSourcePath, pixels, width, height))
	{
		Error("[CEnvironmentGenerator] Failed to load %s\n", pSourcePath);

		return false;
	}

	CUtlVector< CUtlVector<uint8> > faces;
	CEnvironmentFaces::ConvertEquirectToFaces(
		pixels, width, height, faceSize, faces);

	EnvironmentData_t data;
	if (!CEnvironmentPrefilter::Prefilter(faces, faceSize, data))
	{
		Error("[CEnvironmentGenerator] Failed to prefilter the environment\n");

		return false;
	}

	CUtlVector<uint8> bytes;
	if (!CEnvironmentSerializer::Serialize(data, bytes))
	{
		Error("[CEnvironmentGenerator] Failed to serialize environment data\n");

		return false;
	}

	if (!WriteFile(pOutputPath, bytes))
	{
		Error("[CEnvironmentGenerator] Failed to write %s\n", pOutputPath);

		return false;
	}

	return true;
}

bool CEnvironmentGenerator::GenerateDFG(
	const char* pSize,
	const char* pOutputPath)
{
	const int32 size = int32(atoi(pSize));
	if (size <= 0)
	{
		Error("[CEnvironmentGenerator] Invalid size: %s\n", pSize);

		return false;
	}

	CUtlVector<uint8> pixels;
	CubemapIBL::DFG(size, s_DFGSampleCount, pixels);
	if (!stbi_write_png(pOutputPath, size, size, 4, pixels.Base(), size * 4))
	{
		Error("[CEnvironmentGenerator] Failed to write %s\n", pOutputPath);

		return false;
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (argc == 5 && strcmp(argv[1], "environment") == 0)
	{
		return CEnvironmentGenerator::Generate(
			argv[2], argv[3], argv[4]) ? 0 : 1;
	}

	if (argc == 4 && strcmp(argv[1], "dfg") == 0)
	{
		return CEnvironmentGenerator::GenerateDFG(argv[2], argv[3]) ? 0 : 1;
	}

	Error(
		"[CEnvironmentGenerator] Usage: iblgen environment <image> "
		"<face size> <output> or iblgen dfg <size> <output>\n");

	return 1;
}
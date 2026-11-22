/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CUBEMAPSH_H
#define CUBEMAPSH_H

#pragma once

#include "platform.h"
#include "utils/UtlVector.h"

class CVector3;

class CubemapSH
{
public:
	/**
	 * Spherical Harmonics decomposition of the given cubemap
	 * Optionally calculates irradiance by convolving with truncated cos.
	 */
	static void computeSH(
		const CUtlVector< CUtlVector<uint8> >& faces,
		int32 faceSize,
		int32 numBands,
		bool irradiance,
		CVector3* pOutSH);
	/**
	 * Compute spherical harmonics of the irradiance of the given cubemap.
	 * The SH basis are pre-scaled for easier rendering by the shader. The resulting coefficients
	 * are not spherical harmonics (as they're scalled by various factors).
	 */
	static void preprocessSHForShader(CVector3* pSH);
};

#endif // CUBEMAPSH_H

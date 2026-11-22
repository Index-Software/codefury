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

#ifndef CUBEMAPIBL_H
#define CUBEMAPIBL_H

#pragma once

#include "platform.h"
#include "utils/UtlVector.h"

class CEnvironmentFaces;

class CubemapIBL
{
public:
	/**
	 * Computes a roughness LOD using prefiltered importance sampling GGX
	 *
	 * @param outCubemap        the destination cubemap
	 * @param levels            a list of prefiltered lods of the source environment
	 * @param linearRoughness   roughness
	 * @param sampleCount       number of samples for importance sampling
	 */
	static void roughnessFilter(
		const CUtlVector<CEnvironmentFaces>& levels,
		float32 linearRoughness,
		int32 sampleCount,
		CEnvironmentFaces& outCubemap);
	//! Computes the "DFG" term of the "split-sum" approximation and stores it in a 2D image
	static void DFG(
		int32 size,
		int32 sampleCount,
		CUtlVector<uint8>& outPixels);
};

#endif // CUBEMAPIBL_H
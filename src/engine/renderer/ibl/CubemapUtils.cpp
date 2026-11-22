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

#include "renderer/ibl/CubemapUtils.h"
#include <math.h>

static inline float32 sphereQuadrantArea(float32 x, float32 y)
{
	return atan2f(x * y, sqrtf(x * x + y * y + 1.0f));
}

float32 CubemapUtils::solidAngle(int32 dim, int32 u, int32 v)
{
	const float32 invSize = 1.0f / float32(dim);
	const float32 s = (float32(u) + 0.5f) * 2.0f * invSize - 1.0f;
	const float32 t = (float32(v) + 0.5f) * 2.0f * invSize - 1.0f;
	const float32 x0 = s - invSize;
	const float32 y0 = t - invSize;
	const float32 x1 = s + invSize;
	const float32 y1 = t + invSize;

	return sphereQuadrantArea(x0, y0) -
		sphereQuadrantArea(x0, y1) -
		sphereQuadrantArea(x1, y0) +
		sphereQuadrantArea(x1, y1);
}
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

#include "renderer/ibl/CubemapSH.h"
#include "renderer/ibl/Cubemap.h"
#include "renderer/ibl/CubemapUtils.h"
#include "renderer/texture/ColorSpace.h"
#include "mathsfury/Maths.h"
#include "mathsfury/Vector3.h"
#include "../../../../shaders/ShaderConstants.h"
#include <math.h>

static int32 SHindex(int32 m, int32 l)
{
	return l * (l + 1) + m;
}

/*
 * returns n! / d!
 */
static float32 factorial(int32 n, int32 d)
{
	d = d > 1 ? d : 1;
	n = n > 1 ? n : 1;
	float32 result = 1.0f;
	if (n > d)
	{
		for (; n > d; n--)
		{
			result *= float32(n);
		}
	}
	else if (d > n)
	{
		for (; d > n; d--)
		{
			result *= float32(d);
		}
		result = 1.0f / result;
	}

	return result;
}

/*
 * SH scaling factors:
 *  returns sqrt((2*l + 1) / 4*pi) * sqrt( (l-|m|)! / (l+|m|)! )
 */
static float32 Kml(int32 m, int32 l)
{
	const int32 absM = m < 0 ? -m : m;
	const float32 k = float32(2 * l + 1) * factorial(l - absM, l + absM);

	return sqrtf(k) / (2.0f * sqrtf(CMaths::PI));
}

static void Ki(int32 numBands, float32* pOutK)
{
	for (int32 l = 0; l < numBands; l++)
	{
		pOutK[SHindex(0, l)] = Kml(0, l);
		for (int32 m = 1; m <= l; m++)
		{
			const float32 k = sqrtf(2.0f) * Kml(m, l);
			pOutK[SHindex(m, l)] = k;
			pOutK[SHindex(-m, l)] = k;
		}
	}
}

// < cos(theta) > SH coefficients pre-multiplied by 1 / K(0,l)
static float32 computeTruncatedCosSh(int32 l)
{
	if (l == 0)
	{
		return CMaths::PI;
	}
	else if (l == 1)
	{
		return 2.0f * CMaths::PI / 3.0f;
	}
	else if (l & 1)
	{
		return 0.0f;
	}

	const int32 halfL = l / 2;
	const float32 sign = (halfL & 1) ? 1.0f : -1.0f;
	const float32 a0 = sign / float32((l + 2) * (l - 1));
	const float32 a1 = factorial(l, halfL) /
		(factorial(halfL, 1) * float32(1 << l));

	return 2.0f * CMaths::PI * a0 * a1;
}

/*
 * Calculates non-normalized SH bases, i.e.:
 *  m > 0, cos(m*phi)   * P(m,l)
 *  m < 0, sin(|m|*phi) * P(|m|,l)
 *  m = 0, P(0,l)
 */
static void computeShBasis(float32* pOutSHBasis, int32 numBands, const CVector3& s)
{
	// Associated Legendre polynomials by recursion, divided by sin(theta)^|m|
	float32 pml2 = 0.0f;
	float32 pml1 = 1.0f;
	pOutSHBasis[0] = pml1;
	for (int32 l = 1; l < numBands; l++)
	{
		const float32 pml =
			((2.0f * l - 1.0f) * pml1 * s.m_Z - (l - 1.0f) * pml2) / float32(l);
		pml2 = pml1;
		pml1 = pml;
		pOutSHBasis[SHindex(0, l)] = pml;
	}

	float32 pmm = 1.0f;
	for (int32 m = 1; m < numBands; m++)
	{
		pmm = (1.0f - 2.0f * m) * pmm;
		pml2 = pmm;
		pml1 = (2.0f * m + 1.0f) * pmm * s.m_Z;
		pOutSHBasis[SHindex(-m, m)] = pml2;
		pOutSHBasis[SHindex(m, m)] = pml2;
		if (m + 1 < numBands)
		{
			pOutSHBasis[SHindex(-m, m + 1)] = pml1;
			pOutSHBasis[SHindex(m, m + 1)] = pml1;
			for (int32 l = m + 2; l < numBands; l++)
			{
				const float32 pml = ((2.0f * l - 1.0f) * pml1 * s.m_Z -
					(l + m - 1.0f) * pml2) / float32(l - m);
				pml2 = pml1;
				pml1 = pml;
				pOutSHBasis[SHindex(-m, l)] = pml;
				pOutSHBasis[SHindex(m, l)] = pml;
			}
		}
	}

	// (cos(m * phi), sin(m * phi)) * sin(theta)^|m| by recursion on (x, y)
	float32 cm = s.m_X;
	float32 sm = s.m_Y;
	for (int32 m = 1; m <= numBands; m++)
	{
		for (int32 l = m; l < numBands; l++)
		{
			pOutSHBasis[SHindex(-m, l)] *= sm;
			pOutSHBasis[SHindex(m, l)] *= cm;
		}

		const float32 nextCm = cm * s.m_X - sm * s.m_Y;
		const float32 nextSm = sm * s.m_X + cm * s.m_Y;
		cm = nextCm;
		sm = nextSm;
	}
}

// Local adaptation: sequential 6-face loop replaces upstream's JobSystem /
// Cubemap, and byte faces replace its Image (no sRGB LUT upstream).
// Local adaptation: 3 bands are the maximum (CF_IBL_SH_COUNT); upstream
// supports arbitrary bands, these fixed-size buffers do not
void CubemapSH::computeSH(
	const CUtlVector< CUtlVector<uint8> >& faces,
	int32 faceSize,
	int32 numBands,
	bool irradiance,
	CVector3* pOutSH)
{
	if (numBands > 3)
	{
		return;
	}

	float32 srgbToLinear[256];
	for (int32 i = 0; i < 256; i++)
	{
		srgbToLinear[i] = CColorSpace::SRGBToLinear(float32(i) / 255.0f);
	}

	for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
	{
		pOutSH[i] = CVector3(0.0f, 0.0f, 0.0f);
	}

	float32 shBasis[CF_IBL_SH_COUNT];
	const Cubemap cubemap(faceSize);
	for (int32 face = 0; face < 6; face++)
	{
		const uint8* pTexel = faces[face].Base();
		for (int32 y = 0; y < faceSize; y++)
		{
			for (int32 x = 0; x < faceSize; x++, pTexel += 4)
			{
				const float32 solidAngle =
					CubemapUtils::solidAngle(faceSize, x, y);
				const CVector3 color = CVector3(
					srgbToLinear[pTexel[0]],
					srgbToLinear[pTexel[1]],
					srgbToLinear[pTexel[2]]) * solidAngle;

				computeShBasis(
					shBasis,
					numBands,
					cubemap.getDirectionFor(
						Cubemap::Face::Enum(face),
						float32(x) + 0.5f,
						float32(y) + 0.5f));
				for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
				{
					pOutSH[i] += color * shBasis[i];
				}
			}
		}
	}

	float32 k[CF_IBL_SH_COUNT];
	Ki(numBands, k);

	// Convolving with the clamped cosine turns radiance into irradiance
	if (irradiance)
	{
		for (int32 l = 0; l < numBands; l++)
		{
			const float32 truncatedCosSH = computeTruncatedCosSh(l);
			for (int32 m = -l; m <= l; m++)
			{
				k[SHindex(m, l)] *= truncatedCosSH;
			}
		}
	}

	for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
	{
		pOutSH[i] *= k[i];
	}
}

/*
 * This computes the 3-bands SH coefficients of the Cubemap convoluted by the
 * truncated cos(theta) (i.e.: saturate(s.z)), pre-scaled by the reconstruction
 * factors.
 */
void CubemapSH::preprocessSHForShader(CVector3* pSH)
{
	// Coefficient for the polynomial form of the SH functions -- these were taken from
	// "Stupid Spherical Harmonics (SH)" by Peter-Pike Sloan
	// They simply come for expanding the computation of each SH function.
	//
	// To render spherical harmonics we can use the polynomial form, like this:
	//          c += sh[0] * A[0];
	//          c += sh[1] * A[1] * s.y;
	//          c += sh[2] * A[2] * s.z;
	//          c += sh[3] * A[3] * s.x;
	//          c += sh[4] * A[4] * s.y * s.x;
	//          c += sh[5] * A[5] * s.y * s.z;
	//          c += sh[6] * A[6] * (3 * s.z * s.z - 1);
	//          c += sh[7] * A[7] * s.z * s.x;
	//          c += sh[8] * A[8] * (s.x * s.x - s.y * s.y);
	//
	// To save math in the shader, we pre-multiply our SH coefficient by the A[i] factors.
	// Additionally, we include the lambertian diffuse BRDF 1/pi.
	const float32 sqrtPi = sqrtf(CMaths::PI);
	const float32 sqrt3 = sqrtf(3.0f);
	const float32 sqrt5 = sqrtf(5.0f);
	const float32 sqrt15 = sqrtf(15.0f);
	const float32 a[CF_IBL_SH_COUNT] = {
		1.0f / (2.0f * sqrtPi),
		-sqrt3 / (2.0f * sqrtPi),
		sqrt3 / (2.0f * sqrtPi),
		-sqrt3 / (2.0f * sqrtPi),
		sqrt15 / (2.0f * sqrtPi),
		-sqrt15 / (2.0f * sqrtPi),
		sqrt5 / (4.0f * sqrtPi),
		-sqrt15 / (2.0f * sqrtPi),
		sqrt15 / (4.0f * sqrtPi)
	};

	for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
	{
		pSH[i] *= a[i] / CMaths::PI;
	}
}
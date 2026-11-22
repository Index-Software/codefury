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

#include "renderer/ibl/CubemapIBL.h"
#include "renderer/ibl/Cubemap.h"
#include "renderer/ibl/EnvironmentFaces.h"
#include "renderer/ibl/utilities.h"
#include "mathsfury/Maths.h"
#include <math.h>

// be careful w/ the size of this structure, the smaller the better
// File-scope because C++98 forbids local types as template arguments
struct CacheEntry
{
	CVector3 L;
	float32 brdf_NoL;
	float32 lerp;
	uint8 l0;
	uint8 l1;
};

// pdf = D(a) * cosTheta
static CVector3 hemisphereImportanceSampleDggx(
	float32 u,
	float32 v,
	float32 linearRoughness)
{
	const float32 a = linearRoughness;
	const float32 phi = 2.0f * CMaths::PI * u;
	// NOTE: (aa-1) == (a-1)(a+1) produces better fp accuracy
	const float32 cosTheta2 =
		(1.0f - v) / (1.0f + (a + 1.0f) * ((a - 1.0f) * v));
	const float32 cosTheta = sqrtf(cosTheta2);
	const float32 sinTheta = sqrtf(1.0f - cosTheta2);

	return CVector3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
}

static float32 DistributionGGX(float32 noH, float32 linearRoughness)
{
	// NOTE: (aa-1) == (a-1)(a+1) produces better fp accuracy
	const float32 a = linearRoughness;
	const float32 f = (a - 1.0f) * ((a + 1.0f) * (noH * noH)) + 1.0f;

	return (a * a) / (CMaths::PI * f * f);
}

static float32 pow5(float32 x)
{
	const float32 x2 = x * x;

	return x2 * x2 * x;
}

// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
// Height-correlated GGX
static float32 Visibility(float32 noV, float32 noL, float32 linearRoughness)
{
	const float32 a2 = linearRoughness * linearRoughness;
	const float32 ggxL = noV * sqrtf((noL - noL * a2) * noL + a2);
	const float32 ggxV = noL * sqrtf((noV - noV * a2) * noV + a2);

	return 0.5f / (ggxV + ggxL);
}

static CVector3 getTexelDirection(int32 face, int32 x, int32 y, int32 size)
{
	const Cubemap cubemap(size);

	return cubemap.getDirectionFor(
		Cubemap::Face::Enum(face),
		float32(x) + 0.5f,
		float32(y) + 0.5f);
}

// Local patch: deterministic hash replaces upstream's default_random_engine
static float32 getTexelRotation(int32 face, int32 x, int32 y)
{
	uint32 hash = uint32(face) * 73856093u ^
		uint32(x) * 19349663u ^
		uint32(y) * 83492791u;
	hash ^= hash >> 13;
	hash *= 0x5BD1E995u;
	hash ^= hash >> 15;

	return (float32(hash & 0xFFFFu) / 65535.0f * 2.0f - 1.0f) * CMaths::PI;
}

// Er() = mix(x, y, f0), with f90 = 1
static void DFV_Multiscatter(
	float32 noV,
	float32 linearRoughness,
	int32 sampleCount,
	float32& outX,
	float32& outY)
{
	const CVector3 view(sqrtf(1.0f - noV * noV), 0.0f, noV);
	const float32 invCount = 1.0f / float32(sampleCount);
	outX = 0.0f;
	outY = 0.0f;
	for (int32 i = 0; i < sampleCount; i++)
	{
		float32 u;
		float32 v;
		hammersley(uint32(i), invCount, u, v);

		const CVector3 halfVector =
			hemisphereImportanceSampleDggx(u, v, linearRoughness);

		const float32 viewDotHalf = view.Dot(halfVector);
		const float32 voH = CMaths::Clamp(viewDotHalf, 0.0f, 1.0f);
		const float32 noL = CMaths::Clamp(
			2.0f * viewDotHalf * halfVector.m_Z - view.m_Z,
			0.0f,
			1.0f);
		const float32 noH = CMaths::Clamp(halfVector.m_Z, 0.0f, 1.0f);
		if (noL > 0.0f)
		{
			/*
			 * Assuming f90 = 1
			 *   Fc = (1 - V•H)^5
			 *   F(h) = f0*(1 - Fc) + Fc
			 *
			 * f0 and f90 are known at runtime, but thankfully can be factored out, allowing us
			 * to split the integral in two terms and store both terms separately in a LUT.
			 *
			 * At runtime, we can reconstruct Er() exactly as below:
			 *
			 *            4                <v•h>
			 *   DFV.x = --- ∑ Fc V(v, l) ------- <n•l>
			 *            N  h             <n•h>
			 *
			 *            4                <v•h>
			 *   DFV.y = --- ∑    V(v, l) ------- <n•l>
			 *            N  h             <n•h>
			 *
			 *   Er() = (1 - f0) * DFV.x + f0 * DFV.y
			 *
			 *        = mix(DFV.xxx, DFV.yyy, f0)
			 *
			 */
			const float32 visibility =
				Visibility(noV, noL, linearRoughness) * noL * (voH / noH);
			const float32 fc = pow5(1.0f - voH);
			outX += visibility * fc;
			outY += visibility;
		}
	}

	outX *= 4.0f * invCount;
	outY *= 4.0f * invCount;
}

/*
 *
 * Importance sampling GGX - Trowbridge-Reitz
 * ------------------------------------------
 *
 * Important samples are chosen to integrate Dggx() * cos(theta) over the hemisphere.
 *
 * All calculations are made in tangent space, with n = [0 0 1]
 *
 *             l        h (important sample)
 *             .\      /.
 *             . \    / .
 *             .  \  /  .
 *             .   \/   .
 *         ----+---o----+-------> n [0 0 1]
 *     cos(2*theta)     cos(theta)
 *        = n•l            = n•h
 *
 *  v = n
 *  f0 = f90 = 1
 *  V = 1
 *
 *  h is micro facet's normal
 *
 *  l is the reflection of v (i.e.: n) around h  ==>  n•h = l•h = v•h
 *
 *  h = important_sample_ggx()
 *
 *  n•h = [0 0 1]•h = h.z
 *
 *  l = reflect(-n, h)
 *    = 2 * (n•h) * h - n;
 *
 *  n•l = cos(2 * theta)
 *      = cos(theta)^2 - sin(theta)^2
 *      = (n•h)^2 - (1 - (n•h)^2)
 *      = 2(n•h)^2 - 1
 *
 *
 *  pdf() = D(h) <n•h> |J(h)|
 *
 *               1
 *  |J(h)| = ----------
 *            4 <v•h>
 *
 *
 * Pre-filtered importance sampling
 * --------------------------------
 *
 *  see: "Real-time Shading with Filtered Importance Sampling", Jaroslav Krivanek
 *  see: "GPU-Based Importance Sampling, GPU Gems 3", Mark Colbert
 *
 *
 *                   Ωs
 *     lod = log4(K ----)
 *                   Ωp
 *
 *     log4(K) = 1, works well for box filters
 *     K = 4
 *
 *             1
 *     Ωs = ---------, solid-angle of an important sample
 *           N * pdf
 *
 *              4 PI
 *     Ωp ~ --------------, solid-angle of a sample in the base cubemap
 *           texel_count
 *
 *
 * Evaluating the integral
 * -----------------------
 *
 *                    K     fr(h)
 *            Er() = --- ∑ ------- L(h) <n•l>
 *                    N  h   pdf
 *
 * with:
 *
 *            fr() = D(h)
 *
 *                       N
 *            K = -----------------
 *                    fr(h)
 *                 ∑ ------- <n•l>
 *                 h   pdf
 *
 *
 *  It results that:
 *
 *            K           4 <v•h>
 *    Er() = --- ∑ D(h) ------------ L(h) <n•l>
 *            N  h        D(h) <n•h>
 *
 *
 *              K
 *    Er() = 4 --- ∑ L(h) <n•l>
 *              N  h
 *
 *                  N       4
 *    Er() = ------------- --- ∑ V(v) <n•l>
 *             4 ∑ <n•l>    N
 *
 *
 *  +------------------------------+
 *  |          ∑ <n•l> L(h)        |
 *  |  Er() = --------------       |
 *  |            ∑ <n•l>           |
 *  +------------------------------+
 *
 */
// Local adaptation: upstream's mirror, prefilter, updater and userdata
// parameters are dropped; our cubemap is the output and mirror is identity
void CubemapIBL::roughnessFilter(
	const CUtlVector<CEnvironmentFaces>& levels,
	float32 linearRoughness,
	int32 sampleCount,
	CEnvironmentFaces& outCubemap)
{
	const int32 size = outCubemap.GetSize();
	if (linearRoughness == 0.0f)
	{
		// Local adaptation: SampleBilinear replaces upstream's nearest
		// sampleAt(); our cubemap owns texels instead of an Image view
		for (int32 face = 0; face < 6; face++)
		{
			for (int32 y = 0; y < size; y++)
			{
				for (int32 x = 0; x < size; x++)
				{
					const CVector3 normal = getTexelDirection(face, x, y, size);
					const CVector3 color = levels[0].SampleBilinear(normal);
					outCubemap.SetTexel(face, x, y, color);
				}
			}
		}

		return;
	}

	const int32 baseSize = levels[0].GetSize();
	const float32 texelSolidAngle =
		4.0f * CMaths::PI / float32(6 * baseSize * baseSize);
	const int32 maxLevel = levels.Count() - 1;

	CUtlVector<CacheEntry> cache;
	const float32 invCount = 1.0f / float32(sampleCount);
	float32 weight = 0.0f;
	// precompute everything that only depends on the sample #
	for (int32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		float32 u;
		float32 v;
		hammersley(uint32(sampleIndex), invCount, u, v);

		const CVector3 H =
			hemisphereImportanceSampleDggx(u, v, linearRoughness);
		const float32 noH = H.m_Z;
		const float32 noL = 2.0f * noH * noH - 1.0f;
		if (noL > 0.0f)
		{
			const float32 pdf = DistributionGGX(noH, linearRoughness) / 4.0f;
			// K is a LOD bias that allows a bit of overlapping between samples
			const float32 K = 4.0f;
			const float32 omegaS = 1.0f / (float32(sampleCount) * pdf);
			const float32 lod =
				log4(omegaS) - log4(texelSolidAngle) + log4(K);
			const float32 mipLevel =
				CMaths::Clamp(lod, 0.0f, float32(maxLevel));
			const float32 brdf_NoL = noL;
			weight += brdf_NoL;

			CacheEntry entry;
			entry.L = CVector3(
				2.0f * noH * H.m_X,
				2.0f * noH * H.m_Y,
				noL);
			entry.brdf_NoL = brdf_NoL;
			entry.l0 = uint8(mipLevel);
			entry.l1 = uint8(CMaths::Min(maxLevel, int32(entry.l0) + 1));
			entry.lerp = mipLevel - float32(entry.l0);
			cache.AddToTail(entry);
		}
	}

	for (int32 i = 0; i < cache.Count(); i++)
	{
		cache[i].brdf_NoL /= weight;
	}

	// we can sample the cubemap in any order, sort by the weight, it could
	// improve fp precision
	for (int32 i = 1; i < cache.Count(); i++)
	{
		const CacheEntry entry = cache[i];
		int32 j = i - 1;
		for (; j >= 0 && cache[j].brdf_NoL > entry.brdf_NoL; j--)
		{
			cache[j + 1] = cache[j];
		}

		cache[j + 1] = entry;
	}

	for (int32 face = 0; face < 6; face++)
	{
		for (int32 y = 0; y < size; y++)
		{
			for (int32 x = 0; x < size; x++)
			{
				const CVector3 normal = getTexelDirection(face, x, y, size);

				// Tangent frame around the normal, rotated by a per texel angle
				const CVector3 up = fabsf(normal.m_Z) < 0.999f
					? CVector3(0.0f, 0.0f, 1.0f)
					: CVector3(1.0f, 0.0f, 0.0f);

				const CVector3 tangent = CMaths::Normalise(up.Cross(normal));
				const CVector3 bitangent = normal.Cross(tangent);
				const float32 angle = getTexelRotation(face, x, y);
				const float32 cosAngle = cosf(angle);
				const float32 sinAngle = sinf(angle);

				CVector3 radiance(0.0f, 0.0f, 0.0f);
				for (int32 i = 0; i < cache.Count(); i++)
				{
					const CacheEntry& entry = cache[i];
					const CVector3& local = entry.L;
					const float32 rotatedX =
						local.m_X * cosAngle - local.m_Y * sinAngle;
					const float32 rotatedY =
						local.m_X * sinAngle + local.m_Y * cosAngle;

					const CVector3 direction = tangent * rotatedX +
						bitangent * rotatedY +
						normal * local.m_Z;

					// Local adaptation: our cubemap has no upstream-style
					// seamless trilinearFilterAt()
					const CVector3 c0 =
						levels[entry.l0].SampleBilinear(direction);
					const CVector3 c1 =
						levels[entry.l1].SampleBilinear(direction);

					const CVector3 color = c0 + (c1 - c0) * entry.lerp;

					radiance += color * entry.brdf_NoL;
				}

				outCubemap.SetTexel(face, x, y, radiance);
			}
		}
	}
}

void CubemapIBL::DFG(
	int32 size,
	int32 sampleCount,
	CUtlVector<uint8>& outPixels)
{
	outPixels.SetCount(size * size * 4);
	uint8* pDst = outPixels.Base();
	for (int32 y = 0; y < size; y++)
	{
		// coord = sqrt(linear_roughness)
		// Local adaptation: multiscatter-only DFG, and PNG row 0 maps to
		// v=0 on upload, so the upstream V flip does not apply
		const float32 coord = (float32(y) + 0.5f) / float32(size);
		const float32 linearRoughness = coord * coord;
		for (int32 x = 0; x < size; x++, pDst += 4)
		{
			const float32 noV = (float32(x) + 0.5f) / float32(size);
			float32 dfvX;
			float32 dfvY;
			DFV_Multiscatter(noV, linearRoughness, sampleCount, dfvX, dfvY);

			pDst[0] = uint8(CMaths::Clamp(dfvX, 0.0f, 1.0f) * 255.0f + 0.5f);
			pDst[1] = uint8(CMaths::Clamp(dfvY, 0.0f, 1.0f) * 255.0f + 0.5f);
			pDst[2] = 0;
			pDst[3] = 255;
		}
	}
}
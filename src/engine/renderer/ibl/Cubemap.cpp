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

// Local adaptation: C++98 type substitutions only (float3 -> CVector3,
// size_t -> int32, std::sqrt -> sqrtf, ::abs -> fabsf). Bodies, variables
// and comments are upstream's.

#include "renderer/ibl/Cubemap.h"
#include <math.h>

Cubemap::Cubemap(int32 dim) :
	mDimensions(dim),
	mScale(2.0f / float32(dim))
{
}

CVector3 Cubemap::getDirectionFor(Face::Enum face, float32 x, float32 y) const
{
	// map [0, dim] to [-1,1] with (-1,-1) at bottom left
	float32 cx = (x * mScale) - 1;
	float32 cy = 1 - (y * mScale);

	CVector3 dir;
	const float32 l = sqrtf(cx * cx + cy * cy + 1);
	switch (face)
	{
	case Face::PX:  dir = CVector3(  1, cy, -cx ); break;
	case Face::NX:  dir = CVector3( -1, cy,  cx ); break;
	case Face::PY:  dir = CVector3( cx,  1, -cy ); break;
	case Face::NY:  dir = CVector3( cx, -1,  cy ); break;
	case Face::PZ:  dir = CVector3( cx, cy,   1 ); break;
	case Face::NZ:  dir = CVector3(-cx, cy,  -1 ); break;
	}

	return dir * (1 / l);
}

Cubemap::Address Cubemap::getAddressFor(const CVector3& r)
{
	Address addr;
	float sc, tc, ma;
	const float rx = fabsf(r.m_X);
	const float ry = fabsf(r.m_Y);
	const float rz = fabsf(r.m_Z);
	if (rx >= ry && rx >= rz)
	{
		ma = 1.0f / rx;
		if (r.m_X >= 0)
		{
			addr.face = Face::PX;
			sc = -r.m_Z;
			tc = -r.m_Y;
		}
		else
		{
			addr.face = Face::NX;
			sc =  r.m_Z;
			tc = -r.m_Y;
		}
	}
	else if (ry >= rx && ry >= rz)
	{
		ma = 1.0f / ry;
		if (r.m_Y >= 0)
		{
			addr.face = Face::PY;
			sc =  r.m_X;
			tc =  r.m_Z;
		}
		else
		{
			addr.face = Face::NY;
			sc =  r.m_X;
			tc = -r.m_Z;
		}
	}
	else
	{
		ma = 1.0f / rz;
		if (r.m_Z >= 0)
		{
			addr.face = Face::PZ;
			sc =  r.m_X;
			tc = -r.m_Y;
		}
		else
		{
			addr.face = Face::NZ;
			sc = -r.m_X;
			tc = -r.m_Y;
		}
	}

	// ma is guaranteed to be >= sc and tc
	addr.s = (sc * ma + 1.0f) * 0.5f;
	addr.t = (tc * ma + 1.0f) * 0.5f;

	return addr;
}
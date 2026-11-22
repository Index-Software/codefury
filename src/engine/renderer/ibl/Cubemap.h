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

// Local adaptation: this is the subset of Filament libs/ibl Cubemap that
// codefury uses, kept verbatim apart from C++98 type substitutions and the
// enum class -> struct-wrapped enum conversion below. The owning texel
// container and all engine helpers live in EnvironmentFaces.* (MIT).

#ifndef CUBEMAP_H
#define CUBEMAP_H

#pragma once

#include "platform.h"
#include "mathsfury/Vector3.h"

class Cubemap
{
public:
	// Local adaptation: upstream uses "enum class Face : uint8_t"
	struct Face
	{
		enum Enum
		{
			PX = 0,     // left            +----+
			NX,         // right           | PY |
			PY,         // bottom     +----+----+----+----+
			NY,         // top        | NX | PZ | PX | NZ |
			PZ,         // back       +----+----+----+----+
			NZ          // front           | NY |
			            //                 +----+
		};
	};

	// Local adaptation: upstream's Address has in-class member initializers
	struct Address
	{
		Face::Enum face;
		float32 s;
		float32 t;
	};

	explicit Cubemap(int32 dim);

	//! computes a direction vector from a face and a location in pixel in an Image
	CVector3 getDirectionFor(Face::Enum face, float32 x, float32 y) const;

	//! returns the face and texture coordinates of the given direction
	static Address getAddressFor(const CVector3& direction);

private:
	int32 mDimensions;
	float32 mScale;
};

#endif // CUBEMAP_H
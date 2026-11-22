//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SHADOWLAYOUT_H
#define SHADOWLAYOUT_H

#pragma once

#include "platform.h"
#include "shaders/ShaderConstants.h"

#ifdef PLATFORM_PS3
#define CF_POINT_SHADOW_ATLAS
#endif // PLATFORM_PS3

#ifdef PLATFORM_PS3
static const uint32 s_MainShadowMapSize = 1024;
#else // !PLATFORM_PS3
static const uint32 s_MainShadowMapSize = 4096;
#endif // !PLATFORM_PS3

#ifndef CF_POINT_SHADOW_ATLAS
static const uint32 s_PointShadowCubeSize = 2048;
#endif // !CF_POINT_SHADOW_ATLAS

#ifdef CF_POINT_SHADOW_ATLAS
static const int32 s_PointShadowAtlasColumns = 3;
static const int32 s_PointShadowAtlasRows = 2;
static const uint32 s_PointShadowTileSize = 512;
static const uint32 s_PointShadowAtlasWidth =
	s_PointShadowAtlasColumns * s_PointShadowTileSize;
static const uint32 s_PointShadowAtlasHeight =
	s_PointShadowAtlasRows * s_PointShadowTileSize;
static const float32 s_PointShadowTileScale = CF_POINT_SHADOW_TILE_SCALE;
#endif // CF_POINT_SHADOW_ATLAS

#endif // SHADOWLAYOUT_H
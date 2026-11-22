//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SCENE_CGINC
#define SCENE_CGINC

#include "Bindings.cginc"

uniform float4x4 g_LightSpaceMatrix;
uniform float4 g_ShadowMapParams;
uniform float4 g_ShadowBiasParams;
uniform float4 g_Time;
uniform float4 g_ShadowLightDir;
uniform float4 g_ShadowProjX;
uniform float4 g_ShadowProjY;
uniform float4 g_ShadowProjZ;

#endif // SCENE_CGINC
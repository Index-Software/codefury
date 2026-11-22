//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CAMERA_CGINC
#define CAMERA_CGINC

#include "Bindings.cginc"

uniform float4 g_CameraPos;
uniform float4 g_CameraRight;
uniform float4 g_CameraUp;
uniform float4 g_CameraForward;
uniform float g_TanHalfFOV;
uniform float g_AspectRatio;

#endif // CAMERA_CGINC
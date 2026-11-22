//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

struct VertexInput_t
{
    [vk::location(0)] float2 position : POSITION;
};

struct VertexOutput_t
{
    float4 position : SV_Position;
    [vk::location(0)] float2 ndc : TEXCOORD0;
};

[shader("vertex")]
VertexOutput_t vertMain(VertexInput_t input)
{
    VertexOutput_t output;
    output.ndc = input.position;
    output.position = float4(input.position, 0.999, 1.0);

    return output;
}

#include "../Camera.slangh"

[shader("fragment")]
float4 fragMain(VertexOutput_t input) : SV_Target
{
    float3 forward = g_CameraForward.xyz;
    float3 right = g_CameraRight.xyz;
    float3 up = g_CameraUp.xyz;

    float3 rayDir = normalize(
        forward +
        right * input.ndc.x * g_TanHalfFOV * g_AspectRatio +
        up * input.ndc.y * g_TanHalfFOV);

    float t = clamp(rayDir.y, 0.0, 1.0);

    float3 zenith = float3(0.1, 0.3, 0.7);
    float3 horizon = float3(0.6, 0.75, 0.9);

    return float4(lerp(horizon, zenith, t), 1.0);
}
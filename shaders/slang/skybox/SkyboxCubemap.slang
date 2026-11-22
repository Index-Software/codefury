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

#include "../Bindings.slangh"
#include "../Camera.slangh"

[vk::binding(13, 0)] uniform SamplerCube EnvironmentMap;

[shader("fragment")]
float4 fragMain(VertexOutput_t input) : SV_Target
{
    float3 rayDir = normalize(
        g_CameraForward.xyz +
        g_CameraRight.xyz * input.ndc.x * g_TanHalfFOV * g_AspectRatio +
        g_CameraUp.xyz * input.ndc.y * g_TanHalfFOV);

    return EnvironmentMap.Sample(rayDir);
}
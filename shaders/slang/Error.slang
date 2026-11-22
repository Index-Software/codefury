//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Model.slangh"

struct VertexInput_t
{
    [vk::location(0)] float3 position : POSITION;
    [vk::location(2)] float2 texCoord : TEXCOORD0;
};

struct VertexOutput_t
{
    float4 position : SV_Position;
    [vk::location(0)] float2 texCoord : TEXCOORD0;
};

[shader("vertex")]
VertexOutput_t vertMain(VertexInput_t input)
{
    VertexOutput_t output;
    output.position = mul(float4(input.position, 1.0), g_MVP);
    output.texCoord = input.texCoord;

    return output;
}

[vk::binding(0, 0)] uniform Sampler2D AlbedoMap;

[shader("fragment")]
float4 fragMain(VertexOutput_t input) : SV_Target
{
    float4 texColor = AlbedoMap.Sample(input.texCoord);

    return float4(
        lerp(float3(1.0, 0.0, 1.0), texColor.rgb, texColor.a),
        texColor.a);
}
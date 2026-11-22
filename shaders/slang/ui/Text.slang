//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "../Scene.slangh"
#include "../Model.slangh"
#include "../Constants.slangh"
#include "../Math.slangh"

struct VertexInput_t
{
    [vk::location(0)] float3 position : POSITION;
    [vk::location(3)] float3 color : COLOR0;
    [vk::location(2)] float2 texCoord : TEXCOORD0;
    [vk::location(4)] float4 outlineColor : TEXCOORD1;
    [vk::location(6)] float edgeSharpness : TEXCOORD3;
};

struct VertexOutput_t
{
    float4 position : SV_Position;
    [vk::location(0)] float3 color : COLOR0;
    [vk::location(1)] float2 texCoord : TEXCOORD0;
    [vk::location(2)] float4 outlineColor : TEXCOORD1;
    [vk::location(3)] float edgeSharpness : TEXCOORD3;
};

[shader("vertex")]
VertexOutput_t vertMain(VertexInput_t input)
{
    VertexOutput_t output;
    output.position = mul(float4(input.position, 1.0), g_MVP);
    output.color = input.color;
    output.texCoord = input.texCoord;
    output.outlineColor = input.outlineColor;
    output.edgeSharpness = input.edgeSharpness;

    return output;
}

[vk::binding(0, 0)] uniform Sampler2D AlbedoMap;

[shader("fragment")]
float4 fragMain(VertexOutput_t input) : SV_Target
{
    float4 sample = AlbedoMap.Sample(input.texCoord);

    float edgeSharpness = input.edgeSharpness;
    float outlineWidth = input.outlineColor.a * CF_TEXT_OUTLINE_WIDTH_RANGE;

    float value = Median3(sample.r, sample.g, sample.b);
    float alpha = clamp(edgeSharpness * (value - 0.5) + 0.5, 0.0, 1.0);
    float outlineMask = max(0.0, sign(outlineWidth));

    float outlineAlpha = clamp(
        edgeSharpness * (sample.a - 0.5) + outlineWidth + 0.5,
        0.0,
        1.0) * (1.0 - alpha) * outlineMask;

    float finalAlpha = clamp(alpha + outlineAlpha, 0.0, 1.0);
    if (finalAlpha < 0.01)
    {
        discard;
    }
    float3 finalColor = lerp(
        input.color,
        lerp(input.outlineColor.rgb, input.color, alpha),
        outlineMask);

    return float4(finalColor, finalAlpha);
}
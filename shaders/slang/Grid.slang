//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef LIT_PASS_ADD
#define LIT_PASS_BASE
#endif // !LIT_PASS_ADD

#include "Lit.slangh"
#include "Bindings.slangh"
#include "Model.slangh"
#include "Camera.slangh"
#include "Scene.slangh"

static const float s_GridSpacing = 2.0;
static const float s_GridFadeStart = 60.0;
static const float s_GridFadeEnd = 160.0;
static const float3 s_GridLineColor = float3(0.32, 0.32, 0.32);

struct VertexInput_t
{
    [vk::location(0)] float3 position : POSITION;
    [vk::location(1)] float3 normal : NORMAL;
    [vk::location(2)] float2 texCoord : TEXCOORD0;
    [vk::location(3)] float4 color : COLOR0;
};

struct VertexOutput_t
{
    float4 position : SV_Position;
    [vk::location(0)] float3 worldPos : TEXCOORD0;
    [vk::location(1)] float3 normal : TEXCOORD1;
    [vk::location(2)] float3 color : TEXCOORD2;
    [vk::location(3)] float4 lightSpacePos : TEXCOORD3;
};

[shader("vertex")]
VertexOutput_t vertMain(VertexInput_t input)
{
    VertexOutput_t output;
    float4 worldPos = mul(float4(input.position, 1.0), g_Model);
    output.worldPos = worldPos.xyz;
    output.lightSpacePos = mul(float4(worldPos.xyz, 1.0), g_LightSpaceMatrix);
    output.normal = normalize(mul(input.normal, (float3x3)g_Model));
    output.color = input.color.rgb;
    output.position = mul(float4(input.position, 1.0), g_MVP);

    return output;
}

[vk::binding(4, 0)] cbuffer SmoothnessParams
{
    float Smoothness;
};
[vk::binding(5, 0)] cbuffer MetallicParams
{
    float Metallic;
};

[shader("fragment")]
float4 fragMain(VertexOutput_t input) : SV_Target
{
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(g_CameraPos.xyz - input.worldPos);

    float smoothness = clamp(Smoothness, 0.0, 1.0);
    if (Smoothness == 0.0 && Metallic == 0.0)
    {
        smoothness = 0.5;
    }

    float2 coord = input.worldPos.xz / s_GridSpacing;
    float2 grid = abs(frac(coord - 0.5) - 0.5) / fwidth(coord);
    float line = 1.0 - min(min(grid.x, grid.y), 1.0);
    float viewDistance = length(input.worldPos.xz - g_CameraPos.xz);
    line *= 1.0 - smoothstep(s_GridFadeStart, s_GridFadeEnd, viewDistance);

    float3 albedo_srgb = lerp(input.color, s_GridLineColor, line * 0.85);
    float3 finalColor = GetStandardLighting(
        input.worldPos,
        input.lightSpacePos,
        normal,
        viewDir,
        smoothness,
        Metallic,
        albedo_srgb,
        input.position.xy);

    finalColor = LinearToSRGB(finalColor);

    return float4(finalColor, 1.0);
}
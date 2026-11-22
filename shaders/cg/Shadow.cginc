//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SHADOW_CGINC
#define SHADOW_CGINC

#include "Bindings.cginc"
#include "Scene.cginc"

uniform sampler2D g_ShadowMap : CF_BINDING_SHADOW_MAP;
uniform sampler2D g_PointShadowMap : CF_BINDING_POINT_SHADOW_MAP;

float2 GetPointShadowAtlasUV(float3 d, float3 squared, float axisLength)
{
    float3 positive = 1.0 - step(d, float3(0.0, 0.0, 0.0));
    float3 dirSign = positive * 2.0 - 1.0;

    float3 isMax = step(squared.yzx, squared) * step(squared.zxy, squared);
    float xMajor = isMax.x;
    float yMajor = (1.0 - xMajor) * isMax.y;
    float zMajor = 1.0 - xMajor - yMajor;

    float3 major = float3(xMajor, yMajor, zMajor);
    float2 faceNDC = float2(
        dot(
            major * float3(-dirSign.x, 1.0, dirSign.z),
            d.zxx),
        dot(
            major * float3(-1.0, dirSign.y, -1.0),
            d.yzy));
    float2 tile = float2(
        dot(
            major,
            float3(
                1.0 - positive.x,
                2.0 * positive.y,
                2.0 - positive.z)),
        dot(
            major,
            float3(0.0, 1.0 - positive.y, 1.0)));

    float2 ndc = faceNDC * (CF_POINT_SHADOW_TILE_SCALE / axisLength);

    return (tile + 0.5 + ndc * float2(0.5, -0.5)) / float2(3.0, 2.0);
}

float SamplePointShadow(
    float3 fragToLight,
    float farPlane,
    float bias,
    float2 screenPos)
{
    // cgcomp turns abs() into the swizzle yxxx
    float3 squared = fragToLight * fragToLight;
    float axisDepth = sqrt(max(max(squared.x, squared.y), squared.z));

    float n = CF_POINT_SHADOW_NEAR_PLANE;
    float f = farPlane;
    float depth = (f - f * n / (axisDepth - bias)) / (f - n);

    float2 atlasUV = GetPointShadowAtlasUV(fragToLight, squared, axisDepth);

    return tex2D(g_PointShadowMap, float3(atlasUV, depth)).x;
}

float EdgeFade(float2 c)
{
    float2 edge = saturate(min(c, 1.0 - c) * 20.0);

    return edge.x * edge.y;
}

float SampleShadowmap(float3 projCoords, float bias)
{
    float shadow = tex2D(
        g_ShadowMap,
        float3(projCoords.xy, projCoords.z - bias)).x;

    float edgeFade = EdgeFade(projCoords.xy);

    float inside = step(0.0, projCoords.z) * step(projCoords.z, 1.0);

    return lerp(1.0, lerp(1.0, shadow, edgeFade), inside);
}

float GetShadowFade(float shadow, float dotNL, float threshold)
{
    float terminatorFade = smoothstep(0.0, threshold, dotNL);

    return lerp(1.0, shadow, terminatorFade);
}

float4 GetDirectionalShadowPos(float3 worldPos, float3 norm)
{
    float3 lightDir = normalize(g_ShadowLightDir.xyz);
    float dotNL = saturate(dot(norm, lightDir));
    float slope = sqrt(1.0 - dotNL * dotNL) / max(dotNL, 0.001);

    float bias = clamp(
        slope * g_ShadowBiasParams.x * 2.0 + g_ShadowBiasParams.x,
        g_ShadowBiasParams.x,
        g_ShadowBiasParams.x * 10.0);

    float normalBias = clamp(
        slope * g_ShadowBiasParams.y,
        g_ShadowBiasParams.y * 0.4,
        g_ShadowBiasParams.y * 4.0);
    normalBias *= 4.0;

    return float4(worldPos + norm * normalBias, bias);
}

float GetDirectionalShadow(float4 shadowPos)
{
    float4 pos = float4(shadowPos.xyz, 1.0);
    float3 projCoords = float3(
        dot(g_ShadowProjX, pos),
        dot(g_ShadowProjY, pos),
        dot(g_ShadowProjZ, pos));

    return SampleShadowmap(projCoords, shadowPos.w);
}

float GetPointShadow(
    float3 worldPos,
    float3 norm,
    float3 lightPos,
    float dotNL,
    float2 screenPos)
{
    float slope = sqrt(1.0 - dotNL * dotNL) / max(dotNL, 0.001);

    float baseBias = 0.005;
    float2 biases = clamp(
        slope * float2(baseBias * 2.0, baseBias) + float2(baseBias, 0.0),
        float2(baseBias, baseBias * 0.4),
        float2(baseBias * 10.0, baseBias * 4.0));

    float3 offsetWorldPos = worldPos + norm * biases.y;

    return SamplePointShadow(
        offsetWorldPos - lightPos,
        CF_POINT_SHADOW_FAR_PLANE,
        biases.x,
        screenPos);
}

#endif // SHADOW_CGINC
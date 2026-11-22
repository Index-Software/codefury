//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef LIT_CGINC
#define LIT_CGINC

#include "Bindings.cginc"

uniform float4 g_LightData[16];
uniform float g_NumLights;
uniform float g_IsAddPass;
uniform float g_IBLLuminance;
uniform float4 g_SHAr;
uniform float4 g_SHAg;
uniform float4 g_SHAb;
uniform float4 g_SHBr;
uniform float4 g_SHBg;
uniform float4 g_SHBb;
uniform float4 g_SHC;

#define USE_DFG_LUT
#include "pbr/Lighting.cginc"
#include "Shadow.cginc"

void GetLight(
    inout Light light,
    int i,
    float3 worldPos,
    float3 norm)
{
    float3 lightPos = g_LightData[i * 2].xyz;
    float lightIntensity = g_LightData[i * 2].w;
    float3 lightColor = g_LightData[i * 2 + 1].xyz;

    light.colorIntensity = float4(lightColor * lightIntensity, 1.0);
#if defined(LIT_PASS_BASE)
    light.l = normalize(lightPos);
    light.attenuation = 1.0;
#else // LIT_PASS_BASE
    float lightRadius = g_LightData[i * 2 + 1].w;
    float3 toLight = lightPos - worldPos;
    float dist = length(toLight);
    light.l = toLight / max(dist, 0.0001);
    float distSq = dist * dist;
    float radiusSq = max(lightRadius * lightRadius, 0.0001);
    float falloff = saturate(1.0 - (distSq / radiusSq));
    light.attenuation = (falloff * falloff) / (1.0 + 0.1 * distSq);
#endif // LIT_PASS_BASE
    light.NoL = saturate(dot(norm, light.l));
    light.worldPosition = lightPos;
}

float3 GetStandardLighting(
    float3 worldPos,
    float4 shadowPos,
    float3 normal,
    float3 viewDir,
    float smoothness,
    float metallic,
    float3 albedo,
    float2 screenPos)
{
    float3 norm = normal;

#ifdef LIT_PASS_BASE
    float directionalShadow = GetDirectionalShadow(shadowPos);
#endif // LIT_PASS_BASE

    MaterialInputs material;
    initMaterial(material);
    material.baseColor = float4(albedo, 1.0);
    material.roughness = 1.0 - smoothness;
    material.metallic = saturate(metallic);
    material.ambientOcclusion = 1.0;
    material.normal = norm;

    ShadingParams shading;
    shading.position = worldPos;
    shading.view = viewDir;
    shading.normal = norm;
    shading.geometricNormal = norm;
    shading.NoV = clampNoV(max(dot(norm, viewDir), 0.0));
    shading.reflected = reflect(-viewDir, norm);
    shading.attenuation = 1.0;
    shading.tangentToWorld = float3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);

    PixelParams pixel;
    getPixelParams(shading, material, pixel);

#if defined(LIT_PASS_ADD)
    float3 color = float3(0.0, 0.0, 0.0);

    Light light0;
    GetLight(light0, 0, worldPos, norm);
    float shadow = GetPointShadow(
        worldPos,
        norm,
        light0.worldPosition,
        light0.NoL,
        screenPos);
    color += surfaceShading(shading, pixel, light0, shadow);
#else // LIT_PASS_ADD
    float3 color = float3(0.0, 0.0, 0.0);
    evaluateIBL(shading, material, pixel, color);

#if defined(LIT_PASS_BASE)
    if (g_NumLights > 0.0)
    {
        Light light0;
        GetLight(light0, 0, worldPos, norm);
        color += surfaceShading(
            shading,
            pixel,
            light0,
            GetShadowFade(directionalShadow, light0.NoL, 0.35));
    }
#endif // LIT_PASS_BASE
#endif // !LIT_PASS_ADD

    return color;
}

#endif // LIT_CGINC
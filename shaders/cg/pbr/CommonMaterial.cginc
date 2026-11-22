#ifndef COMMON_MATERIAL_CGINC
#define COMMON_MATERIAL_CGINC

#include "CommonMath.cginc"

// min roughness such that (MIN_PERCEPTUAL_ROUGHNESS^4) > 0 in fp16 (i.e. 2^(-14/4), rounded up)
#define MIN_PERCEPTUAL_ROUGHNESS 0.045
#define MIN_ROUGHNESS            0.002025

#define MIN_N_DOT_V 1e-4

float clampNoV(float NoV)
{
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return max(NoV, MIN_N_DOT_V);
}

float3 computeDiffuseColor(float4 baseColor, float metallic)
{
    return baseColor.rgb * (1.0 - metallic);
}

float3 computeF0(float4 baseColor, float metallic, float reflectance)
{
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float computeDielectricF0(float reflectance)
{
    return 0.16 * reflectance * reflectance;
}

float computeMetallicFromSpecularColor(float3 specularColor)
{
    return max3(specularColor);
}

float computeRoughnessFromGlossiness(float glossiness)
{
    return 1.0 - glossiness;
}

float perceptualRoughnessToRoughness(float perceptualRoughness)
{
    return perceptualRoughness * perceptualRoughness;
}

float roughnessToPerceptualRoughness(float roughness)
{
    return sqrt(roughness);
}

float iorToF0(float transmittedIor, float incidentIor)
{
    return sq((transmittedIor - incidentIor) / (transmittedIor + incidentIor));
}

float f0ToIor(float f0)
{
    float r = sqrt(f0);
    return (1.0 + r) / (1.0 - r);
}

float3 f0ClearCoatToSurface(float3 f0)
{
    // Approximation of iorTof0(f0ToIor(f0), 1.5)
    // This assumes that the clear coat layer has an IOR of 1.5
    return saturate(f0 * (f0 * (0.941892 - 0.263008 * f0) + 0.346479) - 0.0285998);
}

#endif // COMMON_MATERIAL_CGINC
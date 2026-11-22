#ifndef SHADING_LIT_CGINC
#define SHADING_LIT_CGINC

#include "BRDF.cginc"
#include "MaterialInputs.cginc"
#include "CommonMaterial.cginc"
#include "CommonShading.cginc"
#include "CommonOcclusion.cginc"
#include "CommonLighting.cginc"
#include "LightIndirect.cginc"
#include "ShadingStandard.cginc"

void getCommonPixelParams(
    const MaterialInputs material,
    inout PixelParams pixel)
{
    float4 baseColor = material.baseColor;

    pixel.diffuseColor = computeDiffuseColor(baseColor, material.metallic);
    float reflectance = computeDielectricF0(material.reflectance);
    pixel.f0 = computeF0(baseColor, material.metallic, reflectance);
}

void getSheenPixelParams(
    const ShadingParams shading,
    const MaterialInputs material,
    inout PixelParams pixel)
{
#if defined(MATERIAL_HAS_SHEEN_COLOR)
    pixel.sheenColor = material.sheenColor;

    float sheenPerceptualRoughness = material.sheenRoughness;
    sheenPerceptualRoughness = clamp(
        sheenPerceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);

    pixel.sheenPerceptualRoughness = sheenPerceptualRoughness;
    pixel.sheenRoughness =
        perceptualRoughnessToRoughness(sheenPerceptualRoughness);
#endif
}

void getClearCoatPixelParams(
    const ShadingParams shading,
    const MaterialInputs material,
    inout PixelParams pixel)
{
#if defined(MATERIAL_HAS_CLEAR_COAT)
    pixel.clearCoat = material.clearCoat;

    // Clamp the clear coat roughness to avoid divisions by 0
    float clearCoatPerceptualRoughness = material.clearCoatRoughness;
    clearCoatPerceptualRoughness =
            clamp(clearCoatPerceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);

    pixel.clearCoatPerceptualRoughness = clearCoatPerceptualRoughness;
    pixel.clearCoatRoughness =
        perceptualRoughnessToRoughness(clearCoatPerceptualRoughness);
#endif
}

void getRoughnessPixelParams(
    const ShadingParams shading,
    const MaterialInputs material,
    inout PixelParams pixel)
{
    float perceptualRoughness = material.roughness;

    // This is used by the refraction code and must be saved before we apply specular AA
    pixel.perceptualRoughnessUnclamped = perceptualRoughness;

#if defined(MATERIAL_HAS_CLEAR_COAT) && defined(MATERIAL_HAS_CLEAR_COAT_ROUGHNESS)
    // This is a hack but it will do: the base layer must be at least as rough
    // as the clear coat layer to take into account possible diffusion by the
    // top layer
    float basePerceptualRoughness = max(perceptualRoughness, pixel.clearCoatPerceptualRoughness);
    perceptualRoughness = lerp(perceptualRoughness, basePerceptualRoughness, pixel.clearCoat);
#endif

    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    pixel.perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.roughness = perceptualRoughnessToRoughness(pixel.perceptualRoughness);
}

void getAnisotropyPixelParams(
    const ShadingParams shading,
    const MaterialInputs material,
    inout PixelParams pixel)
{
#if defined(MATERIAL_HAS_ANISOTROPY)
    float3 direction = material.anisotropyDirection;
    pixel.anisotropy = material.anisotropy;
    pixel.anisotropicT = normalize(mul(shading.tangentToWorld, direction));
    pixel.anisotropicB =
        normalize(cross(shading.geometricNormal, pixel.anisotropicT));
#endif
}

void getEnergyCompensationPixelParams(
    const ShadingParams shading,
    inout PixelParams pixel)
{
    // Pre-filtered DFG term used for image-based lighting
    pixel.dfg = prefilteredDFG(pixel.perceptualRoughness, shading.NoV);

    // Energy compensation for multiple scattering in a microfacet model
    // See "Multiple-Scattering Microfacet BSDFs with the Smith Model"
    pixel.energyCompensation = 1.0 + pixel.f0 * (1.0 / pixel.dfg.yyy - 1.0);

#if defined(MATERIAL_HAS_SHEEN_COLOR)
    pixel.sheenDFG = prefilteredDFG(pixel.sheenPerceptualRoughness, shading.NoV).z;
    pixel.sheenScaling = 1.0 - max3(pixel.sheenColor) * pixel.sheenDFG;
#endif
}

/**
 * Computes all the parameters required to shade the current pixel/fragment.
 * These parameters are derived from the MaterialInputs structure computed
 * by the user's material code.
 *
 * This function is also responsible for discarding the fragment when alpha
 * testing fails.
 */
void getPixelParams(
    const ShadingParams shading,
    const MaterialInputs material,
    out PixelParams pixel)
{
    pixel = (PixelParams)0;
    getCommonPixelParams(material, pixel);
    getSheenPixelParams(shading, material, pixel);
    getClearCoatPixelParams(shading, material, pixel);
    getRoughnessPixelParams(shading, material, pixel);
    getAnisotropyPixelParams(shading, material, pixel);
    getEnergyCompensationPixelParams(shading, pixel);
}

#endif // SHADING_LIT_CGINC
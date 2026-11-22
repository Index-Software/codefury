#ifndef MATERIAL_INPUTS_CGINC
#define MATERIAL_INPUTS_CGINC

#define MATERIAL_CAN_SKIP_LIGHTING

#define MATERIAL_HAS_SHEEN_COLOR
#define MATERIAL_HAS_ANISOTROPY

struct MaterialInputs {
    float4 baseColor;
    float roughness;
    float metallic;
    float reflectance;
    float ambientOcclusion;
    float4 emissive;

    float3 sheenColor;
    float sheenRoughness;

    float clearCoat;
    float clearCoatRoughness;

    float anisotropy;
    float3 anisotropyDirection;

    float3 normal;
};

void initMaterial(out MaterialInputs material)
{
    material.baseColor = float4(1.0, 1.0, 1.0, 1.0);
    material.roughness = 1.0;
    material.metallic = 0.0;
    material.reflectance = 0.5;
    material.ambientOcclusion = 1.0;
    material.emissive = float4(float3(0.0, 0.0, 0.0), 1.0);

    material.sheenColor = float3(0.0, 0.0, 0.0);
    material.sheenRoughness = 0.0;

    material.clearCoat = 1.0;
    material.clearCoatRoughness = 0.0;

    material.anisotropy = 0.0;
    material.anisotropyDirection = float3(1.0, 0.0, 0.0);

    material.normal = float3(0.0, 0.0, 1.0);
}

#endif // MATERIAL_INPUTS_CGINC
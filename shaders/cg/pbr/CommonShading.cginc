#ifndef COMMON_SHADING_CGINC
#define COMMON_SHADING_CGINC

#include "MaterialInputs.cginc"

struct ShadingParams {
    float3x3 tangentToWorld;   // TBN matrix
    float3 position;           // position of the fragment in world space
    float3 view;               // normalized vector from the fragment to the eye
    float3 normal;             // normalized transformed normal, in world space
    float3 geometricNormal;    // normalized geometric normal, in world space
    float3 reflected;          // reflection of view about normal
    float NoV;                 // dot(normal, view), always strictly >= MIN_N_DOT_V

#if defined(MATERIAL_HAS_BENT_NORMAL)
    float3 bentNormal;         // normalized transformed normal, in world space
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
    float3 clearCoatNormal;    // normalized clear coat layer normal, in world space
#endif

    float2 normalizedViewportCoord;

    float attenuation;
    float3 ambient;
    float4 lightmapUV;
};

#endif // COMMON_SHADING_CGINC
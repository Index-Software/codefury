#ifndef COMMON_MATH_CGINC
#define COMMON_MATH_CGINC

//------------------------------------------------------------------------------
// Common math
//------------------------------------------------------------------------------

#define PI                 3.14159265359
#define HALF_PI            1.570796327

#define MEDIUMP_FLT_MAX    65504.0
#define MEDIUMP_FLT_MIN    0.00006103515625

#define FLT_EPS            1e-5

#ifdef TARGET_MOBILE
#define PREVENT_DIV0(n, d, magic)   ((n) / max(d, magic))
#else
#define PREVENT_DIV0(n, d, magic)   ((n) / (d))
#endif

#define atan(x,y)          atan2(y,x)

//------------------------------------------------------------------------------
// Scalar operations
//------------------------------------------------------------------------------

/**
 * Computes x^5 using only multiply operations.
 */
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

/**
 * Computes x^2 as a single multiplication.
 */
float sq(float x) {
    return x * x;
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

/**
 * Returns the maximum component of the specified vector.
 */
float max3(float3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(float2 v) {
    return max(v.x, v.y);
}

float vmax(float3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(float4 v) {
    return max(max(v.x, v.y), max(v.z, v.w));
}

/**
 * Returns the minimum component of the specified vector.
 */
float min3(float3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(float2 v) {
    return min(v.x, v.y);
}

float vmin(float3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(float4 v) {
    return min(min(v.x, v.y), min(v.z, v.w));
}

//------------------------------------------------------------------------------
// Trigonometry
//------------------------------------------------------------------------------

/**
 * Approximates acos(x) with a max absolute error of 9.0x10^-3.
 * Valid in the range -1..1.
 */
float acosFast(float x) {
    // Lagarde 2014, "Inverse trigonometric functions GPU optimization for AMD GCN architecture"
    // This is the approximation of degree 1, with a max absolute error of 9.0x10^-3
    float y = abs(x);
    float p = -0.1565827 * y + 1.570796;
    p *= sqrt(1.0 - y);
    return x >= 0.0 ? p : PI - p;
}

/**
 * Approximates acos(x) with a max absolute error of 9.0x10^-3.
 * Valid only in the range 0..1.
 */
float acosFastPositive(float x) {
    float p = -0.1565827 * x + 1.570796;
    return p * sqrt(1.0 - x);
}

#endif // COMMON_MATH_CGINC
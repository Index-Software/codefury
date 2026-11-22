//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "../Scene.slangh"
#include "../Bindings.slangh"

#include "../Model.slangh"

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
};

[shader("vertex")]
VertexOutput_t vertMain(VertexInput_t input)
{
    VertexOutput_t output;

    float3 worldPos = mul(float4(input.position, 1.0), g_Model).xyz;
    output.position = mul(float4(worldPos, 1.0), g_LightSpaceMatrix);

    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput_t input) : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
#include "common.hlsli"

cbuffer resourceIndices : register(b1, space0)
{
    uint textureIdx;
}

struct PSIn
{
    float4 m_position : SV_POSITION;
    float3 m_normal : NORMAL;
    float2 m_uvs : TEXCOORDS;
    float3 m_tangent : TANGENT;
    float3 m_bitangent : BITANGENT;
};

float4 main_ps( PSIn psIn ) : SV_Target
{
    uint idx = textureIdx;
    float4 col = texture2DResources[idx].Sample(sampler2DResources[0], psIn.m_uvs);
    return col;
}

void depth_ps(PSIn psIn)
{
}
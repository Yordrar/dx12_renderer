#include "common.hlsli"

cbuffer resourceIndices : register(b1, space0)
{
    float textureIdx;
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
    return texture2DResources[textureIdx].Sample(sampler2DResources[0], psIn.m_uvs);
}

float4 depth_ps(PSIn psIn) : SV_Target
{
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
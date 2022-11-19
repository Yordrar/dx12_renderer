#include "common.hlsli"

cbuffer bindlessIndices : register(b2, space0)
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
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
    //return texture2DResources[textureIdx].Sample(textureSampler, psIn.m_uvs);
}

float4 depth_ps(PSIn psIn) : SV_Target
{
    return float4(0.0f, 0.0f, 1.0f, 1.0f);
    //return texture2DResources[textureIdx].Sample(textureSampler, psIn.m_uvs);
}
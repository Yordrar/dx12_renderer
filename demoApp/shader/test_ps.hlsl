#include "common.hlsli"

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
    return texture2DResources[textureIdx].Sample(textureSampler, psIn.m_uvs);
}
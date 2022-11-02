#include "common.hlsli"

struct VSIn
{
    float3 m_position : POSITION;
    float3 m_normal : NORMAL;
    float2 m_uvs : TEXCOORDS;
    float3 m_tangent : TANGENT;
    float3 m_bitangent : BITANGENT;
};

struct VSOut
{
    float4 m_position : SV_POSITION;
    float3 m_normal : NORMAL;
    float2 m_uvs : TEXCOORDS;
    float3 m_tangent : TANGENT;
    float3 m_bitangent : BITANGENT;
};

VSOut main_vs( VSIn vertexData )
{
    VSOut vsOut;
    vsOut.m_position = mul( float4( vertexData.m_position, 1.0f ), viewProj );
    vsOut.m_normal = vertexData.m_normal;
    vsOut.m_uvs = vertexData.m_uvs;
    vsOut.m_tangent = vertexData.m_tangent;
    vsOut.m_bitangent = vertexData.m_bitangent;
    return vsOut;
}

VSOut depth_vs( VSIn vertexData )
{
    return main_vs( vertexData );
}
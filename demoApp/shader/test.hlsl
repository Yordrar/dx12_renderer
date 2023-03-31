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

VSOut commonVertexProcessing(VSIn vertexData)
{
    VSOut vsOut;
    vsOut.m_position = mul(float4(vertexData.m_position, 1.0f), viewProjMatrix);
    vsOut.m_normal = vertexData.m_normal;
    vsOut.m_uvs = vertexData.m_uvs;
    vsOut.m_tangent = vertexData.m_tangent;
    vsOut.m_bitangent = vertexData.m_bitangent;
    return vsOut;
}

#if defined(DEPTH)

VSOut depth_vs( VSIn vertexData )
{
    return commonVertexProcessing( vertexData );
}


float4 depth_ps(VSOut vsOut) : SV_Target
{
    return float4(0.0, 1.0, 0.0, 1.0);
}

#endif

#if defined(MAIN)

VSOut main_vs( VSIn vertexData )
{
    return commonVertexProcessing( vertexData );
}


struct ResourceIndices
{
    uint textureIdx;
};

[earlydepthstencil]
float4 main_ps(VSOut vsOut) : SV_Target
{
    ResourceIndices indices = getBindlessIndicesBuffer<ResourceIndices>();
    float4 col = texture2DResources[indices.textureIdx].Sample(samplerResources[0], vsOut.m_uvs);
    return col;
}

#endif
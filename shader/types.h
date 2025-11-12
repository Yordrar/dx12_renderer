#pragma once

struct CameraData
{
    float4x4 viewProjMatrix;
    float4x4 inverseViewProjMatrix;
    float4 cameraPosition;
};

struct GeometryData
{
    float4x4 modelMatrix;
    float4x4 inverseModelMatrix;
};

struct PointLight
{
    float3 position;
    float3 color;
    float intensity;
};

struct DirectionalLight
{
    float3 direction;
    float3 color;
};

struct SpotLight
{
    float3 direction;
    float3 color;
    float angle;
};

struct StdVertex
{
    float3 m_position : POSITION;
    float3 m_normal : NORMAL;
    float2 m_uvs : TEXCOORDS;
    float3 m_tangent : TANGENT;
    float3 m_bitangent : BITANGENT;
};
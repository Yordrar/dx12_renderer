#include "Scene.h"

#include <Renderer.h>
#include <resource/ResourceManager.h>
#include <Camera.h>

Scene::Scene( wchar_t const* name )
    : m_name( name )
    , m_camera( Camera( std::wstring(m_name + L"_camera").c_str() ) )
{
}

Scene::~Scene()
{

}

void Scene::addMesh( Mesh* mesh )
{
    m_meshes.push_back( mesh );
}

std::vector<Mesh*> const& Scene::getMeshes()
{
    return m_meshes;
}

void Scene::addPointLight(PointLight const& light)
{
    m_pointLights.push_back(light);
    m_pointLightBufferSRV.invalidate();
    recreatePointLightBuffer();
}

void Scene::addDirectionalLight(DirectionalLight const& light)
{
    m_directionalLights.push_back(light);
    m_directionalLightBufferSRV.invalidate();
    recreateDirectionalLightBuffer();
}

void Scene::addSpotLight(SpotLight const& light)
{
    m_spotLights.push_back(light);
    m_spotLightBufferSRV.invalidate();
    recreateSpotLightBuffer();
}

void Scene::recreatePointLightBuffer()
{
    ResourceManager::it().destroyResource(m_pointLightBuffer);

    m_pointLightBuffer = ResourceManager::it().createResource((m_name + L"_pointLightBuffer").c_str(),
        CD3DX12_RESOURCE_DESC::Buffer(m_pointLights.size() * sizeof(PointLight)),
        D3D12_SUBRESOURCE_DATA{ m_pointLights.data(), static_cast<LONG_PTR>(m_pointLights.size() * sizeof(PointLight)), 0 });

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
    {
        .Format = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    };
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = static_cast<UINT>(m_pointLights.size());
    srvDesc.Buffer.StructureByteStride = sizeof(PointLight);
    m_pointLightBufferSRV = ResourceManager::it().getSRV(m_pointLightBuffer, srvDesc);
}

void Scene::recreateDirectionalLightBuffer()
{
    ResourceManager::it().destroyResource(m_directionalLightBuffer);

    m_directionalLightBuffer = ResourceManager::it().createResource((m_name + L"_directionalLightBuffer").c_str(),
        CD3DX12_RESOURCE_DESC::Buffer(m_directionalLights.size() * sizeof(DirectionalLight)),
        D3D12_SUBRESOURCE_DATA{ m_directionalLights.data(), static_cast<LONG_PTR>(m_directionalLights.size() * sizeof(DirectionalLight)), 0 });

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
    {
        .Format = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    };
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = static_cast<UINT>(m_directionalLights.size());
    srvDesc.Buffer.StructureByteStride = sizeof(DirectionalLight);
    m_directionalLightBufferSRV = ResourceManager::it().getSRV(m_directionalLightBuffer, srvDesc);
}

void Scene::recreateSpotLightBuffer()
{
    ResourceManager::it().destroyResource(m_spotLightBuffer);

    m_spotLightBuffer = ResourceManager::it().createResource((m_name + L"_spotLightBuffer").c_str(),
        CD3DX12_RESOURCE_DESC::Buffer(m_spotLights.size() * sizeof(SpotLight)),
        D3D12_SUBRESOURCE_DATA{ m_spotLights.data(), static_cast<LONG_PTR>(m_spotLights.size() * sizeof(SpotLight)), 0 });

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
    {
        .Format = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    };
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = static_cast<UINT>(m_spotLights.size());
    srvDesc.Buffer.StructureByteStride = sizeof(SpotLight);
    m_spotLightBufferSRV = ResourceManager::it().getSRV(m_spotLightBuffer, srvDesc);
}

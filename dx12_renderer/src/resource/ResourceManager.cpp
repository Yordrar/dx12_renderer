#include "ResourceManager.h"

#include <Renderer.h>
#include <resource/ConstantBuffer.h>
#include <resource/Texture.h>
#include <resource/DepthStencilTarget.h>

ResourceManager::ResourceManager()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 1024;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    m_descriptorHeapCbvSrvUav = std::make_unique<DescriptorHeap>( heapDesc );

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    m_descriptorHeapSampler = std::make_unique<DescriptorHeap>( heapDesc );

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_descriptorHeapRtv = std::make_unique<DescriptorHeap>( heapDesc );

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    m_descriptorHeapDsv = std::make_unique<DescriptorHeap>( heapDesc );
}

template<typename resource_type, typename std::is_base_of<IResource, resource_type>::value, typename... Args, typename std::is_constructible<resource_type, Args...>::value>
std::shared_ptr<resource_type> createResource( std::string resourceName, Args&&... args )
{
    m_resources[ resourceName ] = std::make_shared<resource_type>( resourceName, std::forward( args ) );
}

template<typename resource_type, typename std::is_base_of<IResource, resource_type>::value>
std::shared_ptr<resource_type> ResourceManager::getResource( std::string resourceName )
{
    ResourceMap::iterator it = m_resources.find( resourceName );

    if ( it != m_resources.end() )
    {
        return it->second;
    }

    return nullptr;
}
#include "ResourceManager.h"

#include <Renderer.h>
#include <resource/DescriptorHeap.h>
#include <resource/ConstantBuffer.h>
#include <resource/Texture.h>

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

void ResourceManager::createBackbuffer( std::string resourceName, ComPtr<ID3D12Resource> backbuffer )
{
    std::shared_ptr<Texture> backbufferTexture = std::make_shared<Texture>( resourceName, backbuffer );
    m_resources[ resourceName ] = backbufferTexture;

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    m_resources[ resourceName ]->m_descriptorIndex = m_descriptorHeapRtv->addRTV( backbufferTexture->getResource(), &rtvDesc );
    m_resources[ resourceName ]->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapRtv->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                               m_resources[ resourceName ]->m_descriptorIndex,
                                                                               m_descriptorHeapRtv->getIncrementSize() );
    m_resources[ resourceName ]->m_resourceState = D3D12_RESOURCE_STATE_PRESENT;
}

std::shared_ptr<ConstantBuffer> ResourceManager::createConstantBuffer( std::string resourceName, void* data, UINT sizeInBytes )
{
    std::shared_ptr<ConstantBuffer> buffer = std::make_shared<ConstantBuffer>( resourceName, data, sizeInBytes );
    m_resources[ resourceName ] = buffer;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = buffer->getGPUVirtualAddress();
    cbvDesc.SizeInBytes = buffer->getAlignedSizeInBytes();

    m_resources[ resourceName ]->m_descriptorIndex = m_descriptorHeapCbvSrvUav->addCBV( &cbvDesc );
    m_resources[ resourceName ]->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapCbvSrvUav->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                               m_resources[ resourceName ]->m_descriptorIndex,
                                                                               m_descriptorHeapCbvSrvUav->getIncrementSize() );
}

template<typename... Args, typename std::is_constructible<Texture, Args...>::value>
std::shared_ptr<Texture> ResourceManager::createTexture( Args&&... args )
{
    std::shared_ptr<Texture> texture = std::make_shared<Texture>( std::forward(args) );
    m_resources[ resourceName ] = texture;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texture->getFormat();
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 6;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;

    m_resources[ resourceName ]->m_descriptorIndex = m_descriptorHeapCbvSrvUav->addSRV( texture->getResource(), &srvDesc );
    m_resources[ resourceName ]->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapCbvSrvUav->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                               m_resources[ resourceName ]->m_descriptorIndex,
                                                                               m_descriptorHeapCbvSrvUav->getIncrementSize() );
}

template<typename resource_type>
std::shared_ptr<resource_type> ResourceManager::getResource( std::string resourceName )
{
    ResourceMap::iterator it = m_resources.find( resourceName );

    if ( it != m_resources.end() )
    {
        return static_cast<std::shared_ptr<resource_type>>( it->second );
    }
    return nullptr;
}
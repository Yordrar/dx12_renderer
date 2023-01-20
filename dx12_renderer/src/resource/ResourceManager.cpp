#include "ResourceManager.h"

#include <Renderer.h>
#include <resource/Resource.h>
#include <resource/DescriptorHeap.h>
#include <resource/Descriptor.h>
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

Resource* ResourceManager::createResource( std::wstring resourceName, D3D12_RESOURCE_DESC& resourceDesc, D3D12_SUBRESOURCE_DATA& subresourceData )
{
    std::unique_ptr<Resource> newResource = std::make_unique<Resource>( resourceName, resourceDesc, subresourceData );

    if ( newResource->getResourceDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET )
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = newResource->getResourceDesc().Format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        UINT descriptorIndex = m_descriptorHeapRtv->addRTV( newResource->getResource(), &rtvDesc );
        newResource->m_rtv = std::move( std::make_unique<Descriptor>( m_descriptorHeapRtv->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                      descriptorIndex,
                                                                      m_descriptorHeapRtv->getIncrementSize() ) );
    }
    else if ( newResource->getResourceDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL )
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = newResource->getResourceDesc().Format;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        UINT descriptorIndex = m_descriptorHeapDsv->addDSV( newResource->getResource(), &dsvDesc );
        newResource->m_dsv = std::move( std::make_unique<Descriptor>( m_descriptorHeapDsv->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                      descriptorIndex,
                                                                      m_descriptorHeapDsv->getIncrementSize() ) );
    }
    else if ( newResource->getResourceDesc().Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D )
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = newResource->getResourceDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;

        UINT descriptorIndex = m_descriptorHeapCbvSrvUav->addSRV( newResource->getResource(), &srvDesc );
        newResource->m_srv = std::move( std::make_unique<Descriptor>( m_descriptorHeapCbvSrvUav->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                      descriptorIndex,
                                                                      m_descriptorHeapCbvSrvUav->getIncrementSize() ) );
    }
    else if ( newResource->getResourceDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER )
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = newResource->getGPUVirtualAddress();
        cbvDesc.SizeInBytes = newResource->getResource()->GetDesc().Width;

        UINT descriptorIndex = m_descriptorHeapCbvSrvUav->addCBV( &cbvDesc );
        newResource->m_cbv = std::move( std::make_unique<Descriptor>( m_descriptorHeapCbvSrvUav->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                      descriptorIndex,
                                                                      m_descriptorHeapCbvSrvUav->getIncrementSize() ) );
    }

    m_resources[ resourceName ] = std::move( newResource );

    return m_resources[ resourceName ].get();
}

void ResourceManager::destroyResource( std::wstring resourceName )
{
    Resource* resource = getResource( resourceName );
    if ( resource )
    {
        if ( resource->m_cbv )
        {
            m_descriptorHeapCbvSrvUav->removeCBV( resource->m_cbv->getDescriptorIndex() );
        }
        m_resources.erase( resourceName );
    }
}

Resource* ResourceManager::getResource( std::wstring resourceName )
{
    ResourceMap::iterator it = m_resources.find( resourceName );

    if ( it != m_resources.end() )
    {
        return it->second.get();
    }
    return nullptr;
}

void ResourceManager::createSampler( std::wstring resourceName )
{
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    m_descriptorHeapSampler->addSampler( &samplerDesc );
}
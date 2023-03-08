#include "ResourceManager.h"

#include <Renderer.h>
#include <resource/Resource.h>
#include <resource/DescriptorHeap.h>
#include <resource/Descriptor.h>

ResourceManager::ResourceManager()
{
}

Resource* ResourceManager::createResource( wchar_t const* resourceName, D3D12_RESOURCE_DESC const& resDesc, D3D12_SUBRESOURCE_DATA subresourceData )
{
    if ( wmemcmp( resourceName, L"", wcslen( resourceName ) ) == 0 )
    {
        return nullptr;
    }

    D3D12_RESOURCE_DESC resourceDesc = resDesc;

    // For constant buffers, size has to be aligned to 256
    if ( resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER )
    {
        resourceDesc.Width = Resource::getSizeAligned256( static_cast<UINT>( resourceDesc.Width ) );
    }

    std::unique_ptr<Resource> newResource = std::make_unique<Resource>( resourceName, resourceDesc, subresourceData );

    m_resources[ resourceName ] = std::move( newResource );

    return m_resources[ resourceName ].get();
}

Resource* ResourceManager::createResource( wchar_t const* resourceName, ComPtr<ID3D12Resource> resource )
{
    if ( wmemcmp( resourceName, L"", wcslen( resourceName ) ) == 0 )
    {
        return nullptr;
    }

    std::unique_ptr<Resource> newResource = std::make_unique<Resource>( resourceName, resource );

    m_resources[ resourceName ] = std::move( newResource );

    return m_resources[ resourceName ].get();
}

void ResourceManager::destroyResource( wchar_t const* resourceName )
{
    Resource* resource = getResource( resourceName );
    if ( resource )
    {
        m_resources.erase( resourceName );
    }
}

Resource* ResourceManager::getResource( wchar_t const* resourceName )
{
    ResourceMap::iterator it = m_resources.find( resourceName );

    if ( it != m_resources.end() )
    {
        return it->second.get();
    }
    return nullptr;
}

void ResourceManager::createSampler( wchar_t const* resourceName )
{
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    DescriptorHeap::getDescriptorHeapSampler().addSampler( &samplerDesc );
}

void ResourceManager::copyResourcesToGPU( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    PIXScopedEvent( commandList.Get(), PIX_COLOR_DEFAULT, "Copy resources to GPU" );

    std::vector<CD3DX12_RESOURCE_BARRIER> preCopyBarriers;
    std::vector<Resource*> resourcesToCopy;
    for ( ResourceMap::value_type& resource_pair : m_resources )
    {
        if ( resource_pair.second->getNeedsCopyToGPU() )
        {
            CD3DX12_RESOURCE_BARRIER preCopyBarrier = resource_pair.second->getTransitionBarrier( D3D12_RESOURCE_STATE_COPY_DEST );
            preCopyBarriers.push_back( preCopyBarrier );
            resourcesToCopy.push_back( resource_pair.second.get() );
        }
    }

    if ( preCopyBarriers.size() > 0 )
    {
        commandList->ResourceBarrier( static_cast<UINT>( preCopyBarriers.size() ), preCopyBarriers.data() );
    }

    for ( Resource* resource : resourcesToCopy )
    {
        resource->copyDataToGPU( commandList );
    }

    std::vector<CD3DX12_RESOURCE_BARRIER> postCopyBarriers;
    for ( Resource* resource : resourcesToCopy )
    {
        CD3DX12_RESOURCE_BARRIER postCopyBarrier = resource->getTransitionBarrier( D3D12_RESOURCE_STATE_GENERIC_READ );
        postCopyBarriers.push_back( postCopyBarrier );
    }
    if ( postCopyBarriers.size() > 0 )
    {
        commandList->ResourceBarrier( static_cast<UINT>( postCopyBarriers.size() ), postCopyBarriers.data() );
    }
}

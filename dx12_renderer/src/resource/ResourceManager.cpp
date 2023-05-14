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

    // For buffers, size has to be aligned to 256
    if ( resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER )
    {
        resourceDesc.Width = Resource::getSizeAligned256( static_cast<UINT>( resourceDesc.Width ) );
    }

    m_resources[ resourceName ] = std::make_unique<Resource>( resourceName, resourceDesc, subresourceData );

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
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MaxAnisotropy = 0;

    DescriptorHeap::getDescriptorHeapSampler().addSampler( &samplerDesc );
}

void ResourceManager::copyResourcesToGPU( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
    std::vector<Resource*> resourcesToCopy;
    for ( ResourceMap::value_type& resource_pair : m_resources )
    {
        if ( resource_pair.second->getNeedsCopyToGPU() )
        {
            resourcesToCopy.push_back( resource_pair.second.get() );
            if ( resource_pair.second->getResourceState() != D3D12_RESOURCE_STATE_COPY_DEST )
            {
                D3D12_RESOURCE_BARRIER preCopyBarrier = resource_pair.second->getTransitionBarrier( D3D12_RESOURCE_STATE_COPY_DEST );
                preCopyBarriers.push_back( preCopyBarrier );
            }
        }
    }

    if ( preCopyBarriers.size() > 0 )
    {
        PIXScopedEvent(commandList.Get(), PIX_COLOR_DEFAULT, "Copy resources to GPU");

        commandList->ResourceBarrier( static_cast<UINT>( preCopyBarriers.size() ), preCopyBarriers.data() );
    }

    for ( Resource* resource : resourcesToCopy )
    {
        resource->copyDataToGPU( commandList );
    }
}

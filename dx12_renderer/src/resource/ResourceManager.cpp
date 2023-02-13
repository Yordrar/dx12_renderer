#include "ResourceManager.h"

#include <pix3.h>

#include <Renderer.h>
#include <resource/Resource.h>
#include <resource/DescriptorHeap.h>
#include <resource/Descriptor.h>
#include <resource/Texture.h>

ResourceManager::ResourceManager()
{
}

Resource* ResourceManager::createResource( std::wstring resourceName, D3D12_RESOURCE_DESC& resourceDesc, D3D12_SUBRESOURCE_DATA subresourceData )
{
    if ( resourceName == L"" )
    {
        return nullptr;
    }

    // For constant buffers, size has to be aligned to 256
    if ( resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER )
    {
        resourceDesc.Width = Resource::getSizeAligned256( static_cast<UINT>( resourceDesc.Width ) );
    }

    std::unique_ptr<Resource> newResource = std::make_unique<Resource>( resourceName, resourceDesc, subresourceData );

    m_resources[ resourceName ] = std::move( newResource );

    return m_resources[ resourceName ].get();
}

Resource* ResourceManager::createResource( std::wstring resourceName, ComPtr<ID3D12Resource> resource )
{
    assert( resourceName != L"" );

    std::unique_ptr<Resource> newResource = std::make_unique<Resource>( resourceName, resource );

    m_resources[ resourceName ] = std::move( newResource );

    return m_resources[ resourceName ].get();
}

void ResourceManager::destroyResource( std::wstring resourceName )
{
    Resource* resource = getResource( resourceName );
    if ( resource )
    {
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

    DescriptorHeap::getDescriptorHeapSampler().addSampler( &samplerDesc );
}

void ResourceManager::copyResourcesToGPU( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    PIXScopedEvent( commandList.Get(), PIX_COLOR_DEFAULT, "Copy resources to GPU" );

    std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
    std::vector<Resource*> resourcesToCopy;
    for ( ResourceMap::value_type& resource_pair : m_resources )
    {
        if ( resource_pair.second->getNeedsCopyToGPU() )
        {
            std::optional<CD3DX12_RESOURCE_BARRIER> barrier_optional = resource_pair.second->getTransitionBarrier( D3D12_RESOURCE_STATE_COPY_DEST );
            if ( barrier_optional.has_value() )
            {
                barriers.push_back( barrier_optional.value() );
            }
            resourcesToCopy.push_back( resource_pair.second.get() );
        }
    }

    if ( barriers.size() > 0 )
    {
        commandList->ResourceBarrier( static_cast<UINT>( barriers.size() ), barriers.data() );
    }

    for ( Resource* resource : resourcesToCopy )
    {
        resource->copyDataToGPU( commandList );
    }
}

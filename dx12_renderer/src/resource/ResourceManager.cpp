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

void ResourceManager::destroyResource( std::string resourceName )
{
    IResource* resource = getResource<IResource>( resourceName );
    if ( resource )
    {
        m_descriptorHeapCbvSrvUav->removeCBV( resource->m_descriptorIndex );
        m_resources.erase( resourceName );
    }
}

void ResourceManager::createBackbuffer( std::string resourceName, ComPtr<ID3D12Resource> backbuffer )
{
    std::unique_ptr<Texture> backbufferTexture = std::make_unique<Texture>( resourceName, backbuffer );

    backbufferTexture->m_descriptorIndex = m_descriptorHeapRtv->addRTV( backbufferTexture->getResource(), nullptr );
    backbufferTexture->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapRtv->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                     backbufferTexture->m_descriptorIndex,
                                                                     m_descriptorHeapRtv->getIncrementSize() );
    backbufferTexture->m_resourceState = D3D12_RESOURCE_STATE_PRESENT;

    m_resources[ resourceName ] = std::move( backbufferTexture );
}

ConstantBuffer* ResourceManager::createConstantBuffer( std::string resourceName, void* data, UINT sizeInBytes )
{
    std::unique_ptr<ConstantBuffer> buffer = std::make_unique<ConstantBuffer>( resourceName, data, sizeInBytes );

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = buffer->getGPUVirtualAddress();
    cbvDesc.SizeInBytes = buffer->getAlignedSizeInBytes();

    buffer->m_descriptorIndex = m_descriptorHeapCbvSrvUav->addCBV( &cbvDesc );
    buffer->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapCbvSrvUav->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                          buffer->m_descriptorIndex,
                                                          m_descriptorHeapCbvSrvUav->getIncrementSize() );

    m_resources[ resourceName ] = std::move( buffer );

    return getResource<ConstantBuffer>( resourceName );
}

void ResourceManager::createSampler( std::string resourceName )
{
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    m_descriptorHeapSampler->addSampler( &samplerDesc );
}
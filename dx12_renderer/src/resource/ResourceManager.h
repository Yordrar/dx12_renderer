#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <memory>
#include <unordered_map>
#include <type_traits>

#include <Manager.h>
#include <resource/DescriptorHeap.h>
#include <resource/IResource.h>
#include <resource/ConstantBuffer.h>
#include <resource/Texture.h>
#include <resource/TextureCube.h>
#include <resource/TextureSampler.h>

class ResourceManager : public Manager<ResourceManager>
{
    friend class Manager<ResourceManager>;
public:
    ~ResourceManager() = default;

    template<typename resource_type>
    resource_type* getResource( std::wstring resourceName );

    void destroyResource( std::wstring resourceName );

    void createBackbuffer( std::wstring resourceName, ComPtr<ID3D12Resource> backbuffer );

    ConstantBuffer* createConstantBuffer( std::wstring resourceName, void* data, UINT sizeInBytes );

    template<typename... Args>
    Texture* createTexture( std::wstring resourceName, Args&&... args );

    void createSampler( std::wstring resourceName );

    DescriptorHeap const& getCbvSrvUavDescriptorHeap() const { return *m_descriptorHeapCbvSrvUav; }
    DescriptorHeap const& getSamplerDescriptorHeap() const { return *m_descriptorHeapSampler; }
    DescriptorHeap const& getRtvDescriptorHeap() const { return *m_descriptorHeapRtv; }
    DescriptorHeap const& getDsvDescriptorHeap() const { return *m_descriptorHeapDsv; }

private:
    ResourceManager();

    using ResourceMap = std::unordered_map< std::wstring, std::unique_ptr<IResource> >;
    ResourceMap m_resources;

    std::unique_ptr<DescriptorHeap> m_descriptorHeapCbvSrvUav;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapSampler;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapRtv;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapDsv;
};

template<typename resource_type>
inline resource_type* ResourceManager::getResource( std::wstring resourceName )
{
    ResourceMap::iterator it = m_resources.find( resourceName );

    if ( it != m_resources.end() )
    {
        return static_cast<resource_type*>( it->second.get() );
    }
    return nullptr;
}

template<typename... Args>
Texture* ResourceManager::createTexture( std::wstring resourceName, Args&&... args )
{
    std::unique_ptr<Texture> texture = std::make_unique<Texture>( resourceName, std::forward<Args>( args )... );

    if ( texture->getResource()->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET )
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = texture->getFormat();
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        texture->m_descriptorIndex = m_descriptorHeapRtv->addRTV( texture->getResource(), &rtvDesc );
        texture->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapRtv->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                               texture->m_descriptorIndex,
                                                               m_descriptorHeapRtv->getIncrementSize() );
    }
    else if ( texture->getResource()->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL )
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = texture->getFormat();
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        texture->m_descriptorIndex = m_descriptorHeapDsv->addDSV( texture->getResource(), &dsvDesc );
        texture->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapDsv->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                               texture->m_descriptorIndex,
                                                               m_descriptorHeapDsv->getIncrementSize() );
    }
    else
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texture->getFormat();
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;

        texture->m_descriptorIndex = m_descriptorHeapCbvSrvUav->addSRV( texture->getResource(), &srvDesc );
        texture->m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( m_descriptorHeapCbvSrvUav->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                               texture->m_descriptorIndex,
                                                               m_descriptorHeapCbvSrvUav->getIncrementSize() );
    }

    m_resources[ resourceName ] = std::move( texture );

    return getResource<Texture>( resourceName );
}
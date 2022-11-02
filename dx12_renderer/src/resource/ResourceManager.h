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

    void createBackbuffer( std::string resourceName, ComPtr<ID3D12Resource> backbuffer );

    std::shared_ptr<ConstantBuffer> createConstantBuffer( std::string resourceName, void* data, UINT sizeInBytes );

    template<typename... Args>
    std::shared_ptr<Texture> createTexture( std::string resourceName, Args&&... args );

    template<typename resource_type>
    std::shared_ptr<resource_type> getResource( std::string resourceName );

    DescriptorHeap const& getCbvSrvUavDescriptorHeap() const { return *m_descriptorHeapCbvSrvUav; }
    DescriptorHeap const& getSamplerDescriptorHeap() const { return *m_descriptorHeapSampler; }
    DescriptorHeap const& getRtvUavDescriptorHeap() const { return *m_descriptorHeapRtv; }
    DescriptorHeap const& getDsvDescriptorHeap() const { return *m_descriptorHeapDsv; }

private:
    ResourceManager();

    using ResourceMap = std::unordered_map< std::string, std::shared_ptr<IResource> >;
    ResourceMap m_resources;

    std::unique_ptr<DescriptorHeap> m_descriptorHeapCbvSrvUav;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapSampler;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapRtv;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapDsv;
};

template<typename resource_type>
std::shared_ptr<resource_type> ResourceManager::getResource( std::string resourceName )
{
    ResourceMap::iterator it = m_resources.find( resourceName );

    if ( it != m_resources.end() )
    {
        return std::shared_ptr<resource_type>( static_cast<resource_type*>( it->second.get() ) );
    }
    return nullptr;
}

template<typename... Args>
std::shared_ptr<Texture> ResourceManager::createTexture( std::string resourceName, Args&&... args )
{
    std::shared_ptr<Texture> texture = std::make_shared<Texture>( std::forward( args ) );
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

    return texture;
}
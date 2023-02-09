#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <memory>
#include <unordered_map>
#include <type_traits>

#include <Manager.h>
#include <resource/DescriptorHeap.h>
#include <resource/Resource.h>
#include <resource/TextureSampler.h>

class ResourceManager : public Manager<ResourceManager>
{
    friend class Manager<ResourceManager>;
public:
    ~ResourceManager() = default;

    Resource* getResource( std::wstring resourceName );

    Resource* createResource( std::wstring resourceName, D3D12_RESOURCE_DESC& resourceDesc, D3D12_SUBRESOURCE_DATA subresourceData = {nullptr, 0, 0} );
    Resource* createResource( std::wstring resourceName, ComPtr<ID3D12Resource> resource );
    void destroyResource( std::wstring resourceName );

    void createSampler( std::wstring resourceName );

    void copyResourcesToGPU( ComPtr<ID3D12GraphicsCommandList> commandList );

    DescriptorHeap& getCbvSrvUavDescriptorHeap() const { return *m_descriptorHeapCbvSrvUav; }
    DescriptorHeap& getSamplerDescriptorHeap() const { return *m_descriptorHeapSampler; }
    DescriptorHeap& getRtvDescriptorHeap() const { return *m_descriptorHeapRtv; }
    DescriptorHeap& getDsvDescriptorHeap() const { return *m_descriptorHeapDsv; }

private:
    ResourceManager();

    void createDescriptorsForResource( Resource& resource );

    using ResourceMap = std::unordered_map< std::wstring, std::unique_ptr<Resource> >;
    ResourceMap m_resources;

    std::unique_ptr<DescriptorHeap> m_descriptorHeapCbvSrvUav;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapSampler;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapRtv;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapDsv;
};


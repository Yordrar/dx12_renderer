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
public:
    ~ResourceManager() = default;

    void createBackbuffer( std::string resourceName, ComPtr<ID3D12Resource> backbuffer );

    std::shared_ptr<ConstantBuffer> createConstantBuffer( std::string resourceName, void* data, UINT sizeInBytes );

    template<typename... Args, typename std::is_constructible<Texture, Args...>::value>
    std::shared_ptr<Texture> createTexture( Args&&... args );

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

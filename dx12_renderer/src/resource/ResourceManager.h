#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <memory>
#include <unordered_map>
#include <type_traits>

#include <Renderer.h>
#include <Manager.h>
#include <resource/DescriptorHeap.h>
#include <resource/Resource.h>
#include <resource/TextureSampler.h>

class ResourceManager : public Manager<ResourceManager>
{
    friend class Manager<ResourceManager>;
public:
    ~ResourceManager() = default;

    Resource* getResource( wchar_t const* resourceName );
    Resource* getCurrentBackbufferResource() { return getResource( std::wstring( L"backbuffer" + std::to_wstring( Renderer::getCurrentBackbufferIndex() ) ).c_str() ); }

    Resource* createResource( wchar_t const* resourceName, D3D12_RESOURCE_DESC& resourceDesc, D3D12_SUBRESOURCE_DATA subresourceData = {nullptr, 0, 0} );
    Resource* createResource( wchar_t const* resourceName, ComPtr<ID3D12Resource> resource );
    void destroyResource( wchar_t const* resourceName );

    void createSampler( wchar_t const* resourceName );

    void copyResourcesToGPU( ComPtr<ID3D12GraphicsCommandList> commandList );

private:
    ResourceManager();

    using ResourceMap = std::unordered_map< std::wstring, std::unique_ptr<Resource> >;
    ResourceMap m_resources;
};


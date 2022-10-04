#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <memory>
#include <unordered_map>
#include <type_traits>

#include <Manager.h>
#include <resource/IResource.h>
#include <resource/DescriptorHeap.h>

class ResourceManager : Manager<ResourceManager>
{
    friend class Manager<ResourceManager>;
public:

    template<typename resource_type, typename std::is_base_of<IResource, resource_type>::value, typename... Args, typename std::is_constructible<resource_type, Args...>::value>
    std::shared_ptr<resource_type> createResource( std::string resourceName, Args&&... args );

    template<typename resource_type, typename std::is_base_of<IResource, resource_type>::value>
    std::shared_ptr<resource_type> getResource( std::string resourceName );

private:
    ResourceManager();
    ~ResourceManager() = default;

    using ResourceMap = std::unordered_map< std::string, std::shared_ptr<IResource> >;
    ResourceMap m_resources;

    std::unique_ptr<DescriptorHeap> m_descriptorHeapCbvSrvUav;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapSampler;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapRtv;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapDsv;
};

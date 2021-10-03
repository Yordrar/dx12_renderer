#include "ResourceManager.h"

#include <memory>

#include <Renderer.h>

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager::ResourceManager()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 128;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    m_descriptorHeapSrvCbvUav = std::make_unique<DescriptorHeap>( heapDesc );

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    m_descriptorHeapSampler = std::make_unique<DescriptorHeap>( heapDesc );

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    m_descriptorHeapRtv = std::make_unique<DescriptorHeap>( heapDesc );

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    m_descriptorHeapDsv = std::make_unique<DescriptorHeap>( heapDesc );
}

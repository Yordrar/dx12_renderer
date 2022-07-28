#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <queue>

#include <resource/ResourceHandle.h>

class DescriptorHeap
{
public:
    DescriptorHeap( D3D12_DESCRIPTOR_HEAP_DESC heapDesc );

    ComPtr<ID3D12DescriptorHeap> getHeap() const { return m_heap; }
    UINT getIncrementSize() const { return m_incrementSize; }

    ResourceHandle addSRV( ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC* srv );
    ResourceHandle addCBV( D3D12_CONSTANT_BUFFER_VIEW_DESC* cbv );
    ResourceHandle addUAV( ComPtr<ID3D12Resource> resource, D3D12_UNORDERED_ACCESS_VIEW_DESC* uav );

    ResourceHandle addSampler( D3D12_SAMPLER_DESC* samplerDesc );

    ResourceHandle addRTV( ComPtr<ID3D12Resource> resource, D3D12_RENDER_TARGET_VIEW_DESC* rtv );

    ResourceHandle addDSV( ComPtr<ID3D12Resource> resource, D3D12_DEPTH_STENCIL_VIEW_DESC* dsv );

private:
    UINT getNextSlot();

    ComPtr<ID3D12DescriptorHeap> m_heap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_type;
    UINT m_incrementSize;
    std::queue<UINT> m_freeSlots;
    UINT m_lastFreeSlot;
    UINT m_numSlots;
};


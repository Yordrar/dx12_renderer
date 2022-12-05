#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <queue>

class DescriptorHeap
{
public:
    DescriptorHeap( D3D12_DESCRIPTOR_HEAP_DESC heapDesc );

    ComPtr<ID3D12DescriptorHeap> getHeap() const { return m_heap; }
    UINT getIncrementSize() const { return m_incrementSize; }

    UINT addSRV( ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC* srv );
    UINT addCBV( D3D12_CONSTANT_BUFFER_VIEW_DESC* cbv );
    void removeCBV( UINT index );
    UINT addUAV( ComPtr<ID3D12Resource> resource, D3D12_UNORDERED_ACCESS_VIEW_DESC* uav );

    UINT addSampler( D3D12_SAMPLER_DESC* samplerDesc );

    UINT addRTV( ComPtr<ID3D12Resource> resource, D3D12_RENDER_TARGET_VIEW_DESC* rtv );

    UINT addDSV( ComPtr<ID3D12Resource> resource, D3D12_DEPTH_STENCIL_VIEW_DESC* dsv );

private:
    UINT getNextSlot();

    ComPtr<ID3D12DescriptorHeap> m_heap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_type;
    UINT m_incrementSize;
    std::queue<UINT> m_freeSlots;
    UINT m_nextFreeSlot;
    UINT m_numSlots;
};


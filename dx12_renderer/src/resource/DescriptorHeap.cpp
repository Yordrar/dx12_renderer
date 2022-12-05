#include "DescriptorHeap.h"

#include <assert.h>

#include <Renderer.h>

DescriptorHeap::DescriptorHeap( D3D12_DESCRIPTOR_HEAP_DESC heapDesc )
    : m_type(heapDesc.Type)
    , m_incrementSize(Renderer::device()->GetDescriptorHandleIncrementSize(heapDesc.Type))
    , m_nextFreeSlot(0)
    , m_numSlots(heapDesc.NumDescriptors)
{
    Renderer::device()->CreateDescriptorHeap( &heapDesc, IID_PPV_ARGS( m_heap.GetAddressOf() ) );
}

UINT DescriptorHeap::addSRV( ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC* srv )
{
    assert( m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    UINT slot = getNextSlot();

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle( m_heap->GetCPUDescriptorHandleForHeapStart(), slot, m_incrementSize );
    Renderer::device()->CreateShaderResourceView( resource.Get(), srv, handle );

    return slot;
}

UINT DescriptorHeap::addCBV( D3D12_CONSTANT_BUFFER_VIEW_DESC* cbv )
{
    assert( m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    UINT slot = getNextSlot();

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle( m_heap->GetCPUDescriptorHandleForHeapStart(), slot, m_incrementSize );
    Renderer::device()->CreateConstantBufferView( cbv, handle );

    return slot;
}

void DescriptorHeap::removeCBV( UINT index )
{
    m_freeSlots.push( index );
}

UINT DescriptorHeap::addUAV( ComPtr<ID3D12Resource> resource, D3D12_UNORDERED_ACCESS_VIEW_DESC* uav )
{
    assert( m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    UINT slot = getNextSlot();

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle( m_heap->GetCPUDescriptorHandleForHeapStart(), slot, m_incrementSize );
    Renderer::device()->CreateUnorderedAccessView( resource.Get(), nullptr, uav, handle );

    return slot;
}

UINT DescriptorHeap::addSampler( D3D12_SAMPLER_DESC* samplerDesc )
{
    assert( m_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );

    UINT slot = getNextSlot();

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle( m_heap->GetCPUDescriptorHandleForHeapStart(), slot, m_incrementSize );
    Renderer::device()->CreateSampler( samplerDesc, handle );

    return slot;
}

UINT DescriptorHeap::addRTV( ComPtr<ID3D12Resource> resource, D3D12_RENDER_TARGET_VIEW_DESC* rtv )
{
    assert( m_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

    UINT slot = getNextSlot();

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle( m_heap->GetCPUDescriptorHandleForHeapStart(), slot, m_incrementSize );
    Renderer::device()->CreateRenderTargetView( resource.Get(), rtv, handle );

    return slot;
}

UINT DescriptorHeap::addDSV( ComPtr<ID3D12Resource> resource, D3D12_DEPTH_STENCIL_VIEW_DESC* dsv )
{
    assert( m_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV );

    UINT slot = getNextSlot();

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle( m_heap->GetCPUDescriptorHandleForHeapStart(), slot, m_incrementSize );
    Renderer::device()->CreateDepthStencilView( resource.Get(), dsv, handle );

    return slot;
}

UINT DescriptorHeap::getNextSlot()
{
    UINT slot = 0;

    if ( m_freeSlots.empty() )
    {
        slot = m_nextFreeSlot++;
    }
    else
    {
        slot = m_freeSlots.front();
        m_freeSlots.pop();
    }

    return slot;
}

#include "Profiler.h"

#include <d3dx12.h>

#include <Renderer.h>

Profiler::Profiler()
    : m_queryHeap( nullptr )
    , m_readBackResource( nullptr )
{
    D3D12_QUERY_HEAP_DESC queryHeapDesc = { };
    queryHeapDesc.Count = 1024;
    queryHeapDesc.NodeMask = 0;
    queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    Renderer::device()->CreateQueryHeap( &queryHeapDesc, IID_PPV_ARGS( m_queryHeap.GetAddressOf() ) );

    CD3DX12_HEAP_PROPERTIES readBackResourceHeapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_READBACK, 0, 0 );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(1024);
    Renderer::device()->CreateCommittedResource( &readBackResourceHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &resourceDesc,
                                                 D3D12_RESOURCE_STATE_COMMON,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_readBackResource.GetAddressOf() ) );
}
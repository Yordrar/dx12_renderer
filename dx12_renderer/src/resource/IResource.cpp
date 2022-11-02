#include "IResource.h"

#include <cassert>

#include <Renderer.h>

IResource::~IResource()
{
}

void IResource::transitionToState( ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState )
{
    if ( newState != m_resourceState )
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition
        (
            m_resource.Get(),
            m_resourceState,
            newState
        );
        commandList->ResourceBarrier( 1, &barrier );
        m_resourceState = newState;
    }
}

void IResource::initInternalResources( CD3DX12_RESOURCE_DESC resourceDesc )
{
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
    Renderer::device()->CreateCommittedResource( &heapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &resourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_resource.GetAddressOf() ) );

    CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    Renderer::device()->CreateCommittedResource( &uploadHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &CD3DX12_RESOURCE_DESC::Buffer( GetRequiredIntermediateSize( m_resource.Get(), 0, 1 ) ),
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );
}

UINT IResource::getSizeAligned256( UINT sizeInBytes )
{
    return ( sizeInBytes + 255 ) & ~255;
}
#include "IResource.h"

#include <cassert>

#include <Renderer.h>

IResource::IResource()
    : m_resource( nullptr )
    , m_intermediateUploadBuffer( nullptr )
    , m_needsCopyToGPU( false )
    , m_descriptorIndex( 0 )
    , m_descriptor()
    , m_resourceState( D3D12_RESOURCE_STATE_COMMON )
{
}

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

void IResource::copyDataToGPU( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    if ( m_subresourceData.size() > 0 && m_needsCopyToGPU )
    {
        transitionToState( commandList, D3D12_RESOURCE_STATE_COPY_DEST );
        UpdateSubresources<1>( commandList.Get(), m_resource.Get(), m_intermediateUploadBuffer.Get(), 0, 0, 1, &m_subresourceData[ 0 ] );
        transitionToState( commandList, D3D12_RESOURCE_STATE_GENERIC_READ );
    }
}

void IResource::initInternalResources( CD3DX12_RESOURCE_DESC resourceDesc )
{
    FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    CD3DX12_CLEAR_VALUE clearValue;
    CD3DX12_CLEAR_VALUE* clearValuePtr = nullptr;
    
    if ( resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET )
    {
        clearValue = CD3DX12_CLEAR_VALUE( resourceDesc.Format, clearColor );
        clearValuePtr = &clearValue;
    }
    else if ( resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL )
    {
        clearValue = CD3DX12_CLEAR_VALUE( resourceDesc.Format, 1.0f, 0.0f );
        clearValuePtr = &clearValue;
    }

    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
    Renderer::device()->CreateCommittedResource( &heapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &resourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 clearValuePtr,
                                                 IID_PPV_ARGS( m_resource.GetAddressOf() ) );

    CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    Renderer::device()->CreateCommittedResource( &uploadHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &CD3DX12_RESOURCE_DESC::Buffer( GetRequiredIntermediateSize( m_resource.Get(), 0, 1 ) ),
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );

    m_resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
}

UINT IResource::getSizeAligned256( UINT sizeInBytes )
{
    return ( sizeInBytes + 255 ) & ~255;
}
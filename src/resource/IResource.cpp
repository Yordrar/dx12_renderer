#include "IResource.h"

#include <cassert>

IResource::~IResource()
{
}

UINT IResource::getSlot()
{
    return m_handle.getSlot();
}

void IResource::updateResource( Renderer::RenderContext& context )
{
    assert( m_subresourceData.size() > 0 );
    context.m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_resource.Get(),
                                                                                        D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                                        D3D12_RESOURCE_STATE_COPY_DEST ) );
    UpdateSubresources( context.m_commandList.Get(),
                        m_resource.Get(),
                        m_intermediateUploadBuffer.Get(), 0,
                        0, m_subresourceData.size(),
                        m_subresourceData.data() );
    context.m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_resource.Get(),
                                                                                        D3D12_RESOURCE_STATE_COPY_DEST,
                                                                                        D3D12_RESOURCE_STATE_GENERIC_READ ) );
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
#include "Resource.h"

#include <cassert>

#include <Renderer.h>

Resource::Resource( std::wstring& name, D3D12_RESOURCE_DESC& resourceDesc, D3D12_SUBRESOURCE_DATA& subresourceData )
    : m_resource( nullptr )
    , m_intermediateUploadBuffer( nullptr )
    , m_subresourceData( subresourceData )
    , m_srv( nullptr )
    , m_cbv( nullptr )
    , m_uav( nullptr )
    , m_rtv( nullptr )
    , m_dsv( nullptr )
    , m_resourceState( D3D12_RESOURCE_STATE_GENERIC_READ )
    , m_needsCopyToGPU( subresourceData.pData != nullptr )
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
        clearValue = CD3DX12_CLEAR_VALUE( resourceDesc.Format, 1.0f, 0 );
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
    setDebugName( name );
}

Resource::Resource( std::wstring& name, D3D12_RESOURCE_DESC& resourceDesc )
    : Resource(name, resourceDesc, D3D12_SUBRESOURCE_DATA{nullptr, 0, 0})
{
    m_needsCopyToGPU = false;
}

Resource::Resource( std::wstring& name, ComPtr<ID3D12Resource> resource )
    : m_resource( resource )
    , m_intermediateUploadBuffer( nullptr )
    , m_subresourceData( D3D12_SUBRESOURCE_DATA{ nullptr, 0, 0 } )
    , m_srv( nullptr )
    , m_cbv( nullptr )
    , m_uav( nullptr )
    , m_rtv( nullptr )
    , m_dsv( nullptr )
    , m_resourceState( D3D12_RESOURCE_STATE_COMMON )
    , m_needsCopyToGPU( false )
{

    CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    Renderer::device()->CreateCommittedResource( &uploadHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &CD3DX12_RESOURCE_DESC::Buffer( GetRequiredIntermediateSize( m_resource.Get(), 0, 1 ) ),
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );
    setDebugName( name );
}

Resource::~Resource()
{

}

std::optional<CD3DX12_RESOURCE_BARRIER> Resource::getTransitionBarrier( D3D12_RESOURCE_STATES newState )
{
    if ( newState != m_resourceState )
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition
        (
            m_resource.Get(),
            m_resourceState,
            newState
        );

        m_resourceState = newState;

        return std::optional<CD3DX12_RESOURCE_BARRIER>( barrier );
    }

    return std::optional<CD3DX12_RESOURCE_BARRIER>();
}

void Resource::copyDataToGPU( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    assert( m_resourceState == D3D12_RESOURCE_STATE_COPY_DEST );
    assert( m_subresourceData.pData );
    UpdateSubresources<1>( commandList.Get(), m_resource.Get(), m_intermediateUploadBuffer.Get(), 0, 0, 1, &m_subresourceData );
    m_needsCopyToGPU = false;
}

void Resource::setDebugName( std::wstring& debugName )
{
    m_resource->SetName( debugName.c_str() );
}
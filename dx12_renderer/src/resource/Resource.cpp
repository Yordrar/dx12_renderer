#include "Resource.h"

#include <cassert>
#include <memory>

#include <Renderer.h>
#include <resource/Descriptor.h>
#include <resource/DescriptorHeap.h>

Resource::Resource( wchar_t const* name, D3D12_RESOURCE_DESC const& resourceDesc, D3D12_SUBRESOURCE_DATA const& subresourceData )
    : m_name( name )
    , m_resource( nullptr )
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

    CD3DX12_HEAP_PROPERTIES heapProperties( D3D12_HEAP_TYPE_DEFAULT );
    Renderer::device()->CreateCommittedResource( &heapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &resourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 clearValuePtr,
                                                 IID_PPV_ARGS( m_resource.GetAddressOf() ) );

    CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer( GetRequiredIntermediateSize( m_resource.Get(), 0, 1 ) );
    CD3DX12_HEAP_PROPERTIES uploadHeapProperties( D3D12_HEAP_TYPE_UPLOAD );
    Renderer::device()->CreateCommittedResource( &uploadHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &uploadBufferResourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );
    setDebugName( name );
}

Resource::Resource( wchar_t const* name, ComPtr<ID3D12Resource> resource )
    : m_name( name )
    , m_resource( resource )
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
    CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer( GetRequiredIntermediateSize( m_resource.Get(), 0, 1 ) );
    Renderer::device()->CreateCommittedResource( &uploadHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &uploadBufferResourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );
    setDebugName( name );
}

Resource::~Resource()
{
    if ( m_cbv )
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().removeDescriptor( *m_cbv );
    }

    if ( m_srv )
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().removeDescriptor( *m_srv );
    }

    if ( m_uav )
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().removeDescriptor( *m_uav );
    }

    if ( m_rtv )
    {
        DescriptorHeap::getDescriptorHeapRtv().removeDescriptor( *m_rtv );
    }

    if ( m_dsv )
    {
        DescriptorHeap::getDescriptorHeapDsv().removeDescriptor( *m_dsv );
    }
}

Descriptor const* Resource::getShaderResourceView()
{
    if ( !m_srv )
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = getResourceDesc().Format;
        switch ( getResourceDesc().Dimension )
        {
            case D3D12_RESOURCE_DIMENSION_BUFFER:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srvDesc.Buffer.FirstElement = 0;
                srvDesc.Buffer.NumElements = 1;
                srvDesc.Buffer.StructureByteStride = getResourceDesc().Width;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.PlaneSlice = 0;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                break;
            default:
                break;
        }
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addSRV( getResource(), &srvDesc );
        m_srv = std::move( std::make_unique<Descriptor>( DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                         descriptorIndex,
                                                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() ) );
    }

    return m_srv.get();
}

Descriptor const* Resource::getConstantBufferView()
{
    if ( !m_cbv && ( getResourceDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ) )
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = getGPUVirtualAddress();
        cbvDesc.SizeInBytes = static_cast<UINT>( getResourceDesc().Width );

        UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addCBV( &cbvDesc );
        m_cbv = std::move( std::make_unique<Descriptor>( DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                         descriptorIndex,
                                                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() ) );
    }

    return m_cbv.get();
}

Descriptor const* Resource::getUniformAccessView()
{
    return nullptr;
}

Descriptor const* Resource::getRenderTargetView()
{
    if ( !m_rtv && ( getResourceDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ) )
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = getResourceDesc().Format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        UINT descriptorIndex = DescriptorHeap::getDescriptorHeapRtv().addRTV( getResource(), &rtvDesc );
        m_rtv = std::move( std::make_unique<Descriptor>( DescriptorHeap::getDescriptorHeapRtv().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                         descriptorIndex,
                                                         DescriptorHeap::getDescriptorHeapRtv().getIncrementSize() ) );
    }

    return m_rtv.get();
}

Descriptor const* Resource::getDepthStencilView()
{
    if ( !m_dsv && ( getResourceDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ) )
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = getResourceDesc().Format;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        UINT descriptorIndex = DescriptorHeap::getDescriptorHeapDsv().addDSV( getResource(), &dsvDesc );
        m_dsv = std::move( std::make_unique<Descriptor>( DescriptorHeap::getDescriptorHeapDsv().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                         descriptorIndex,
                                                         DescriptorHeap::getDescriptorHeapDsv().getIncrementSize() ) );
    }

    return m_dsv.get();
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

void Resource::setDebugName( wchar_t const* debugName )
{
    m_resource->SetName( debugName );
}
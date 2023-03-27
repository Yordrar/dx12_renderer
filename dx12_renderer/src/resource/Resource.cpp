#include "Resource.h"

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
    , m_uavs()
    , m_rtv( nullptr )
    , m_dsv( nullptr )
    , m_resourceState( D3D12_RESOURCE_STATE_COMMON )
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
                                                 D3D12_RESOURCE_STATE_COMMON,
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

    m_uavs.resize( getResourceDesc().MipLevels );
}

Resource::Resource( wchar_t const* name, ComPtr<ID3D12Resource> resource )
    : m_name( name )
    , m_resource( resource )
    , m_intermediateUploadBuffer( nullptr )
    , m_subresourceData( D3D12_SUBRESOURCE_DATA{ nullptr, 0, 0 } )
    , m_srv( nullptr )
    , m_cbv( nullptr )
    , m_uavs()
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

    m_uavs.resize( getResourceDesc().MipLevels );
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

    for ( std::unique_ptr<Descriptor> const& uav : m_uavs )
    {
        if ( uav )
        {
            DescriptorHeap::getDescriptorHeapCbvSrvUav().removeDescriptor( *uav );
        }
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
                srvDesc.Buffer.StructureByteStride = static_cast<UINT>( getResourceDesc().Width );
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                srvDesc.Texture1D.MipLevels = getResourceDesc().MipLevels;
                srvDesc.Texture1D.MostDetailedMip = 0;
                srvDesc.Texture1D.ResourceMinLODClamp = 0;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = getResourceDesc().MipLevels;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.PlaneSlice = 0;
                srvDesc.Texture2D.ResourceMinLODClamp = 0;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                srvDesc.Texture3D.MipLevels = getResourceDesc().MipLevels;
                srvDesc.Texture3D.MostDetailedMip = 0;
                srvDesc.Texture3D.ResourceMinLODClamp = 0;
                break;
            default:
                break;
        }
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addSRV( getResource(), &srvDesc );
        m_srv = std::move( std::make_unique<Descriptor>( this,
                                                         Descriptor::Type::ShaderResourceView,
                                                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                         descriptorIndex,
                                                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() ) );
    }

    return m_srv.get();
}

Descriptor const* Resource::getConstantBufferView()
{
    if ( !m_cbv && getResourceDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER )
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = getGPUVirtualAddress();
        cbvDesc.SizeInBytes = static_cast<UINT>( getResourceDesc().Width );

        UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addCBV( &cbvDesc );
        m_cbv = std::move( std::make_unique<Descriptor>( this,
                                                         Descriptor::Type::ConstantBufferView,
                                                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                         descriptorIndex,
                                                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() ) );
    }

    return m_cbv.get();
}

Descriptor const* Resource::getUnorderedAccessView( UINT mipSlice )
{
    assert( mipSlice < getResourceDesc().MipLevels );
    if ( !m_uavs[ mipSlice ] && ( getResourceDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) )
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = getResourceDesc().Format;
        switch ( getResourceDesc().Dimension )
        {
            case D3D12_RESOURCE_DIMENSION_BUFFER:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.FirstElement = 0;
                uavDesc.Buffer.NumElements = 1;
                uavDesc.Buffer.StructureByteStride = static_cast<UINT>( getResourceDesc().Width );
                uavDesc.Buffer.CounterOffsetInBytes = 0;
                uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                uavDesc.Texture1D.MipSlice = mipSlice;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = mipSlice;
                uavDesc.Texture2D.PlaneSlice = 0;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.MipSlice = mipSlice;
                uavDesc.Texture3D.FirstWSlice = 0;
                uavDesc.Texture3D.WSize = 0;
                break;
            default:
                break;
        }

        UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addUAV( getResource(), &uavDesc );
        m_uavs[ mipSlice ] = std::move( std::make_unique<Descriptor>( this,
                                                                      Descriptor::Type::UniformAccessView,
                                                                      DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                                      descriptorIndex,
                                                                      DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() ) );
    }

    return m_uavs[ mipSlice ].get();
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
        m_rtv = std::move( std::make_unique<Descriptor>( this,
                                                         Descriptor::Type::RenderTargetView,
                                                         DescriptorHeap::getDescriptorHeapRtv().getHeap()->GetCPUDescriptorHandleForHeapStart(),
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
        m_dsv = std::move( std::make_unique<Descriptor>( this,
                                                         Descriptor::Type::DepthStencilView,
                                                         DescriptorHeap::getDescriptorHeapDsv().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                         descriptorIndex,
                                                         DescriptorHeap::getDescriptorHeapDsv().getIncrementSize() ) );
    }

    return m_dsv.get();
}

CD3DX12_RESOURCE_BARRIER Resource::getTransitionBarrier( D3D12_RESOURCE_STATES newState )
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition
    (
        m_resource.Get(),
        m_resourceState,
        newState
    );

    m_resourceState = newState;

    return barrier;
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